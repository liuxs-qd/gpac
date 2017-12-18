/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Authors: Jean Le Feuvre
 *			Copyright (c) Telecom ParisTech 2000-2017
 *					All rights reserved
 *
 *  This file is part of GPAC / force reframer filter
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <gpac/avparse.h>
#include <gpac/constants.h>
#include <gpac/filters.h>

GF_Err reframer_configure_pid(GF_Filter *filter, GF_FilterPid *pid, Bool is_remove)
{
	GF_FilterPid *opid = gf_filter_pid_get_udta(pid);

	if (is_remove) {
		if (opid)
			gf_filter_pid_remove(opid);
		return GF_OK;
	}
	if (! gf_filter_pid_check_caps(pid))
		return GF_NOT_SUPPORTED;

	if (opid) {
		gf_filter_pid_reset_properties(opid);
	} else {
		opid = gf_filter_pid_new(filter);
		gf_filter_pid_set_udta(pid, opid);
	}
	gf_filter_pid_copy_properties(opid, pid);
	return GF_OK;
}



GF_Err reframer_process(GF_Filter *filter)
{
	u32 i, count = gf_filter_get_ipid_count(filter);

	for (i=0; i<count; i++) {
		GF_FilterPid *ipid = gf_filter_get_ipid(filter, i);
		GF_FilterPid *opid = ipid ? gf_filter_pid_get_udta(ipid) : NULL;
		assert(ipid);
		assert(opid);

		while (1) {
			GF_FilterPacket *pck = gf_filter_pid_get_packet(ipid);
			if (!pck) {
				if (gf_filter_pid_is_eos(ipid))
					gf_filter_pid_set_eos(opid);
				break;
			}
			gf_filter_pck_forward(pck, opid);
			gf_filter_pid_drop_packet(ipid);
		}
	}
	return GF_OK;
}

//input caps are explicit only for reframer, so we don't load the reframer during resolution process
static const GF_FilterCapability ReframerInputs[] =
{
	CAP_EXC_UINT(GF_PROP_PID_STREAM_TYPE, GF_STREAM_FILE),
	CAP_EXC_BOOL(GF_PROP_PID_UNFRAMED, GF_TRUE),
	{}
};


static const GF_FilterCapability ReframerOutputs[] =
{
	CAP_INC_UINT(GF_PROP_PID_STREAM_TYPE, GF_STREAM_AUDIO),
	CAP_INC_UINT(GF_PROP_PID_STREAM_TYPE, GF_STREAM_VISUAL),
	CAP_INC_UINT(GF_PROP_PID_STREAM_TYPE, GF_STREAM_SCENE),
	CAP_INC_UINT(GF_PROP_PID_STREAM_TYPE, GF_STREAM_OD),
	//we don't accep files as input so don't output them
	CAP_EXC_UINT(GF_PROP_PID_STREAM_TYPE, GF_STREAM_FILE),
	//we don't produce RAW streams - this will avoid loading the filter for compositor/other raw access
	CAP_EXC_UINT(GF_PROP_PID_CODECID, GF_CODECID_RAW),
	{}
};


GF_FilterRegister ReframerRegister = {
	.name = "reframer",
	.description = "Passthrough filter forcing reframers instatiation for file to file operations",
	.explicit_only = 1,
	INCAPS(ReframerInputs),
	OUTCAPS(ReframerOutputs),
	.configure_pid = reframer_configure_pid,
	.process = reframer_process,
};


const GF_FilterRegister *reframer_register(GF_FilterSession *session)
{
	return &ReframerRegister;
}