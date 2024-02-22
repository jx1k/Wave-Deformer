/* Wave Deformer for Cinema 4D
 * 
 * Copyright (C) 2016 Johannes Kollender - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the AGPL-3.0 license.
 *
 * You should have received a copy of the AGPL-3.0 license with
 * this file. If not, please visit:
 * https://github.com/jx1k/Wave-Deformer/blob/main/LICENSE
 */
 
#include "c4d.h"

// forward declarations
Bool RegisterWaveDeformer(void);

Bool PluginStart(void)
{
	if (!RegisterWaveDeformer()) return FALSE;
	GePrint("----------------------------------------------------------------------");
	GePrint("Wave Deformer v0.1 by Johannes Kollender (www.kollender.com)");
	GePrint("Based on original source code by Vidar Nelson of www.creativetools.se");
	GePrint("----------------------------------------------------------------------");

	return TRUE;
}

void PluginEnd(void)
{
}

Bool PluginMessage(LONG id, void *data)
{
	//use the following lines to set a plugin priority
	//
	switch (id)
	{
		case C4DPL_INIT_SYS:
			if (!resource.Init()) return FALSE; // don't start plugin without resource
			return TRUE;

		case C4DMSG_PRIORITY: 
			return TRUE;
	}

	return FALSE;
}
