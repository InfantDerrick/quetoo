/*
 * Copyright(c) 1997-2001 id Software, Inc.
 * Copyright(c) 2002 The Quakeforge Project.
 * Copyright(c) 2006 Quetoo.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "qlight.h"

_Bool antialias = false;
_Bool indirect = false;

vec_t brightness = 1.0;
vec_t saturation = 1.0;
vec_t contrast = 1.0;

int16_t luxel_size = DEFAULT_BSP_LUXEL_SIZE;
int16_t patch_size = DEFAULT_BSP_PATCH_SIZE;

int32_t indirect_bounces = 1;
int32_t indirect_bounce = 0;

/**
 * @brief
 */
int32_t Light_PointLeafnum(const vec3_t point) {
	int32_t nodenum;

	nodenum = 0;
	while (nodenum >= 0) {
		bsp_node_t *node = &bsp_file.nodes[nodenum];
		bsp_plane_t *plane = &bsp_file.planes[node->plane_num];
		vec_t dist = DotProduct(point, plane->normal) - plane->dist;
		if (dist > 0) {
			nodenum = node->children[0];
		} else {
			nodenum = node->children[1];
		}
	}

	return -nodenum - 1;
}

/**
 * @brief
 */
_Bool Light_PointPVS(const vec3_t org, byte *pvs) {

	if (!bsp_file.vis_data_size) {
		memset(pvs, 0xff, (bsp_file.num_leafs + 7) / 8);
		return true;
	}

	const bsp_leaf_t *leaf = &bsp_file.leafs[Light_PointLeafnum(org)];
	if (leaf->cluster == -1) {
		return false; // in solid leaf
	}

	Cm_ClusterPVS(leaf->cluster, pvs);
	return true;
}

/**
 * @brief
 */
_Bool Light_InPVS(const vec3_t p1, const vec3_t p2) {
	byte pvs[MAX_BSP_LEAFS >> 3];

	const int32_t leaf1 = Cm_PointLeafnum(p1, 0);
	const int32_t leaf2 = Cm_PointLeafnum(p2, 0);

	const int32_t area1 = Cm_LeafArea(leaf1);
	const int32_t area2 = Cm_LeafArea(leaf2);

	if (!Cm_AreasConnected(area1, area2)) {
		return false; // a door blocks sight
	}

	const int32_t cluster1 = Cm_LeafCluster(leaf1);
	const int32_t cluster2 = Cm_LeafCluster(leaf2);

	Cm_ClusterPVS(cluster1, pvs);

	if ((pvs[cluster2 >> 3] & (1 << (cluster2 & 7))) == 0) {
		return false;
	}

	return true;
}

// we use the collision detection facilities for lighting
static cm_bsp_model_t *bsp_models[MAX_BSP_MODELS];

/**
 * @brief
 */
void Light_Trace(cm_trace_t *trace, const vec3_t start, const vec3_t end, int32_t mask) {

	vec_t frac = FLT_MAX;

	for (int32_t i = 0; i < Cm_NumModels(); i++) {

		const cm_trace_t tr = Cm_BoxTrace(start, end, NULL, NULL, bsp_models[i]->head_node, mask);
		if (tr.fraction < frac) {
			frac = tr.fraction;
			*trace = tr;
		}
	}
}

/**
 * @brief
 */
static void LightWorld(void) {

	if (bsp_file.num_nodes == 0 || bsp_file.num_faces == 0) {
		Com_Error(ERROR_FATAL, "Empty map\n");
	}

	// load the map for tracing
	bsp_models[0] = Cm_LoadBspModel(bsp_name, NULL);

	for (int32_t i = 1; i < Cm_NumModels(); i++) {
		bsp_models[i] = Cm_Model(va("*%d", i));
	}

	// resolve global lighting parameters from worldspawn

	GList *entities = Cm_LoadEntities(bsp_file.entity_string);
	const cm_entity_t *e = entities->data;

	if (brightness == 1.0) {
		if (Cm_EntityVector(e, "brightness", &brightness, 1) == 1) {
			Com_Verbose("Brightness: %g\n", brightness);
		}
	}

	if (saturation == 1.0) {
		if (Cm_EntityVector(e, "saturation", &saturation, 1) == 1) {
			Com_Verbose("Saturation: %g\n", saturation);
		}
	}

	if (contrast == 1.0) {
		if (Cm_EntityVector(e, "contrast", &contrast, 1) == 1) {
			Com_Verbose("Contrast: %g\n", contrast);
		}
	}

	if (luxel_size == DEFAULT_BSP_LUXEL_SIZE) {
		vec_t v;
		if (Cm_EntityVector(e, "luxel_size", &v, 1) == 1) {
			luxel_size = v;
			Com_Verbose("Luxel size: %d\n", luxel_size);
		}
	}

	if (patch_size == DEFAULT_BSP_PATCH_SIZE) {
		vec_t v;
		if (Cm_EntityVector(e, "patch_size", &v, 1) == 1) {
			patch_size = v;
			Com_Verbose("Patch size: %d\n", patch_size);
		}
	}

	// turn each face into a single patch
	BuildPatches(entities);

	// subdivide patches to the desired resolution
	SubdividePatches();

	// build lightmaps
	BuildLightmaps();

	// create direct lights out of patches and entities
	BuildDirectLights(entities);

	// calculate direct lighting
	Work(DirectLighting, bsp_file.num_faces);

	if (indirect) {
		for (indirect_bounce = 0; indirect_bounce < indirect_bounces; indirect_bounce++) {

			// create indirect lights from directly lit patches
			BuildIndirectLights();

			// calculate indirect lighting
			Work(IndirectLighting, bsp_file.num_faces);

			// free the indirect light sources
			Mem_FreeTag(MEM_TAG_LIGHT);
		}
	}

	// finalize it and write it out
	bsp_file.lightmap_data_size = 0;
	Bsp_AllocLump(&bsp_file, BSP_LUMP_LIGHTMAPS, MAX_BSP_LIGHTING);

	// merge direct and indirect lighting, normalize all samples
	Work(FinalizeLighting, bsp_file.num_faces);

	g_list_free_full(entities, Mem_Free);

	// free the lightmaps
	Mem_FreeTag(MEM_TAG_LIGHTMAP);

	// free the patches
	Mem_FreeTag(MEM_TAG_PATCH);
}

/**
 * @brief
 */
int32_t LIGHT_Main(void) {

	Com_Print("\n----- LIGHT -----\n\n");

	const time_t start = time(NULL);

	LoadBSPFile(bsp_name, BSP_LUMPS_ALL);

	if (!bsp_file.vis_data_size) {
		Com_Error(ERROR_FATAL, "No VIS information\n");
	}

	LoadMaterials(va("materials/%s.mat", map_base), ASSET_CONTEXT_TEXTURES, NULL);

	BuildTextureColors();

	LightWorld();

	FreeTextureColors();

	FreeMaterials();

	WriteBSPFile(va("maps/%s.bsp", map_base));

	const time_t end = time(NULL);
	const time_t duration = end - start;
	Com_Print("\nLIGHT Time: ");
	if (duration > 59) {
		Com_Print("%d Minutes ", (int32_t) (duration / 60));
	}
	Com_Print("%d Seconds\n", (int32_t) (duration % 60));

	return 0;
}
