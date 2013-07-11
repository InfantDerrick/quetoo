/*
 * Copyright(c) 1997-2001 Id Software, Inc.
 * Copyright(c) 2002 The Quakeforge Project.
 * Copyright(c) 2006 Quake2World.
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

#include "cg_local.h"
#include "game/default/bg_pmove.h"

/*
 * @brief Trace wrapper for Pm_Move.
 */
static c_trace_t Cg_PredictMovement_Trace(const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs) {
	return cgi.Trace(start, end, mins, maxs, 0, MASK_PLAYER_SOLID);
}

/*
 * @brief Debug messages for Pm_Move.
 */
static void Cg_PredictMovement_Debug(const char *msg) {
	cgi.Debug_("Cg_PredictMovement: ", "%s", msg);
}

/*
 * @brief Run the latest movement commands through the player movement code
 * locally, using the resulting origin and angles to reduce perceived latency.
 */
void Cg_PredictMovement(void) {
	pm_move_t pm;

	const uint32_t current = cgi.net_chan->outgoing_sequence;
	uint32_t ack = cgi.net_chan->incoming_acknowledged;

	// if we are too far out of date, just freeze
	if (current - ack >= CMD_BACKUP) {
		cgi.Debug("Exceeded CMD_BACKUP\n");
		return;
	}

	// copy current state to into the move
	memset(&pm, 0, sizeof(pm));
	pm.s = cgi.client->frame.ps.pm_state;

	pm.ground_entity = cgi.client->predicted_ground_entity;

	pm.PointContents = cgi.PointContents;
	pm.Trace = Cg_PredictMovement_Trace;

	pm.Debug = Cg_PredictMovement_Debug;

	// run frames
	while (++ack <= current) {
		const uint32_t frame = ack & CMD_MASK;
		const user_cmd_t *cmd = &cgi.client->cmds[frame];

		if (!cmd->msec)
			continue;

		pm.cmd = *cmd;
		Pm_Move(&pm);

		// for each movement, check for stair interaction and interpolate
		const vec_t step = pm.s.origin[2] * 0.125 - cgi.client->predicted_origin[2];
		const vec_t astep = fabs(step);

		if ((pm.s.pm_flags & PMF_ON_STAIRS) && astep >= 8.0 && astep <= 16.0) {
			cgi.client->predicted_step_time = cgi.client->time;
			cgi.client->predicted_step_lerp = 100 * (astep / 16.0);
			cgi.client->predicted_step = step;
		}

		// save for debug checking
		VectorCopy(pm.s.origin, cgi.client->predicted_origins[frame]);
	}

	// copy results out for rendering
	UnpackPosition(pm.s.origin, cgi.client->predicted_origin);
	UnpackPosition(pm.s.view_offset, cgi.client->predicted_offset);

	UnpackAngles(pm.cmd.angles, cgi.client->predicted_angles);

	cgi.client->predicted_ground_entity = pm.ground_entity;
}
