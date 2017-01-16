IF [%1] == [] GOTO INVALID
IF [%2] == [] GOTO INVALID
IF [%1] == [""] GOTO INVALID
IF [%2] == [""] GOTO INVALID

set quetoo_folder=%1
set build_name=%2

robocopy "bin/%build_name%/" "%quetoo_folder%/bin/" quetoo-update* /E /NJH /NJS /FP /NP /V | findstr /v "*EXTRA File"

GOTO DONE

:INVALID
echo "No build input folder. Gonedy."

:DONE