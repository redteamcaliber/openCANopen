#include <linux/can.h>

#include "tst.h"
#include "canopen/sdo.h"

int set_get_cs()
{
	struct can_frame frame = { .data = { 0xff } };
	sdo_set_cs(&frame, 0); ASSERT_INT_EQ(0, sdo_get_cs(&frame));
	sdo_set_cs(&frame, 1); ASSERT_INT_EQ(1, sdo_get_cs(&frame));
	sdo_set_cs(&frame, 2); ASSERT_INT_EQ(2, sdo_get_cs(&frame));
	sdo_set_cs(&frame, 3); ASSERT_INT_EQ(3, sdo_get_cs(&frame));
	return 0;
}

int set_get_segment_size()
{
	struct can_frame frame = { .data = { 0xff } };
	sdo_set_segment_size(&frame, 0);
	ASSERT_INT_EQ(0, sdo_get_segment_size(&frame));
	sdo_set_segment_size(&frame, 1);
	ASSERT_INT_EQ(1, sdo_get_segment_size(&frame));
	sdo_set_segment_size(&frame, 2);
	ASSERT_INT_EQ(2, sdo_get_segment_size(&frame));
	sdo_set_segment_size(&frame, 3);
	ASSERT_INT_EQ(3, sdo_get_segment_size(&frame));
	sdo_set_segment_size(&frame, 4);
	ASSERT_INT_EQ(4, sdo_get_segment_size(&frame));
	sdo_set_segment_size(&frame, 5);
	ASSERT_INT_EQ(5, sdo_get_segment_size(&frame));
	sdo_set_segment_size(&frame, 6);
	ASSERT_INT_EQ(6, sdo_get_segment_size(&frame));
	sdo_set_segment_size(&frame, 7);
	ASSERT_INT_EQ(7, sdo_get_segment_size(&frame));
	return 0;
}

int set_get_abort_code()
{
	struct can_frame frame = { 0 };
	sdo_set_abort_code(&frame, 0xdeadbeef);
	ASSERT_INT_EQ(0xdeadbeef, sdo_get_abort_code(&frame));
	return 0;
}

int sdo_srv_dl_init_ok()
{
	struct sdo_srv_dl_sm sm = { .dl_state = SDO_SRV_DL_START };
	struct can_frame frame_in = { 0 };
	struct can_frame frame_out = { 0 };
	
	sdo_set_cs(&frame_in, SDO_CCS_DL_INIT_REQ);
	strcpy((char*)&frame_in.data[SDO_MULTIPLEXER_IDX], "ab");
	ASSERT_INT_EQ(SDO_SRV_DL_SEG, sdo_srv_dl_sm_init(&sm, &frame_in,
							 &frame_out));
	ASSERT_INT_EQ(SDO_SRV_DL_SEG, sm.dl_state);
	ASSERT_INT_EQ(SDO_SCS_DL_INIT_RES, sdo_get_cs(&frame_out));
	ASSERT_STR_EQ("ab", (char*)&frame_out.data[SDO_MULTIPLEXER_IDX]);

	return 0;
}

int sdo_srv_dl_init_failed_cs()
{
	struct sdo_srv_dl_sm sm = { .dl_state = SDO_SRV_DL_START };
	struct can_frame frame_in = { 0 };
	struct can_frame frame_out = { 0 };
	
	sdo_set_cs(&frame_in, SDO_CCS_DL_SEG_REQ);
	ASSERT_INT_EQ(SDO_SRV_DL_ABORT, sdo_srv_dl_sm_init(&sm, &frame_in,
							 &frame_out));
	ASSERT_INT_EQ(SDO_SRV_DL_ABORT, sm.dl_state);
	ASSERT_INT_EQ(SDO_SCS_DL_ABORT, sdo_get_cs(&frame_out));
	ASSERT_INT_EQ(SDO_ABORT_INVALID_CS, sdo_get_abort_code(&frame_out));

	return 0;
}

int sdo_srv_dl_init_remote_abort()
{
	struct sdo_srv_dl_sm sm = { .dl_state = SDO_SRV_DL_START };
	struct can_frame frame_in = { 0 };
	
	sdo_set_cs(&frame_in, SDO_CCS_DL_ABORT);
	ASSERT_INT_EQ(SDO_SRV_DL_INIT, sdo_srv_dl_sm_init(&sm, &frame_in,
							 NULL));
	ASSERT_INT_EQ(SDO_SRV_DL_INIT, sm.dl_state);

	return 0;
}

int sdo_srv_dl_seg_ok()
{
	struct sdo_srv_dl_sm sm = { .dl_state = SDO_SRV_DL_START };
	struct can_frame frame_in = { 0 }, frame_out = { 0 };
	
	sdo_set_cs(&frame_in, SDO_CCS_DL_SEG_REQ);
	ASSERT_INT_EQ(SDO_SRV_DL_SEG_TOGGLED, sdo_srv_dl_sm_seg(&sm, &frame_in,
								&frame_out, 0));
	ASSERT_INT_EQ(SDO_SRV_DL_SEG_TOGGLED, sm.dl_state);
	ASSERT_INT_EQ(SDO_SCS_DL_SEG_RES, sdo_get_cs(&frame_out));
	ASSERT_FALSE(sdo_is_toggled(&frame_out));
	ASSERT_FALSE(sdo_is_end_segment(&frame_out));

	return 0;
}

int sdo_srv_dl_seg_toggled_ok()
{
	struct sdo_srv_dl_sm sm = { .dl_state = SDO_SRV_DL_START };
	struct can_frame frame_in = { 0 }, frame_out = { 0 };
	
	sdo_set_cs(&frame_in, SDO_CCS_DL_SEG_REQ);
	sdo_toggle(&frame_in);
	ASSERT_INT_EQ(SDO_SRV_DL_SEG, sdo_srv_dl_sm_seg(&sm, &frame_in,
							&frame_out, 1));
	ASSERT_INT_EQ(SDO_SRV_DL_SEG, sm.dl_state);
	ASSERT_INT_EQ(SDO_SCS_DL_SEG_RES, sdo_get_cs(&frame_out));
	ASSERT_TRUE(sdo_is_toggled(&frame_out));
	ASSERT_FALSE(sdo_is_end_segment(&frame_out));

	return 0;
}

int sdo_srv_dl_seg_end_ok()
{
	struct sdo_srv_dl_sm sm = { .dl_state = SDO_SRV_DL_START };
	struct can_frame frame_in = { 0 }, frame_out = { 0 };
	
	sdo_set_cs(&frame_in, SDO_CCS_DL_SEG_REQ);
	sdo_end_segment(&frame_in);
	ASSERT_INT_EQ(SDO_SRV_DL_DONE, sdo_srv_dl_sm_seg(&sm, &frame_in,
							 &frame_out, 0));
	ASSERT_INT_EQ(SDO_SRV_DL_DONE, sm.dl_state);
	ASSERT_INT_EQ(SDO_SCS_DL_SEG_RES, sdo_get_cs(&frame_out));
	ASSERT_FALSE(sdo_is_toggled(&frame_out));
	ASSERT_TRUE(sdo_is_end_segment(&frame_out));

	return 0;
}

int sdo_srv_dl_seg_toggled_end_ok()
{
	struct sdo_srv_dl_sm sm = { .dl_state = SDO_SRV_DL_START };
	struct can_frame frame_in = { 0 }, frame_out = { 0 };
	
	sdo_set_cs(&frame_in, SDO_CCS_DL_SEG_REQ);
	sdo_toggle(&frame_in);
	sdo_end_segment(&frame_in);
	ASSERT_INT_EQ(SDO_SRV_DL_DONE, sdo_srv_dl_sm_seg(&sm, &frame_in,
							 &frame_out, 1));
	ASSERT_INT_EQ(SDO_SRV_DL_DONE, sm.dl_state);
	ASSERT_INT_EQ(SDO_SCS_DL_SEG_RES, sdo_get_cs(&frame_out));
	ASSERT_TRUE(sdo_is_toggled(&frame_out));
	ASSERT_TRUE(sdo_is_end_segment(&frame_out));

	return 0;
}

int sdo_srv_dl_seg_failed_cs()
{
	struct sdo_srv_dl_sm sm = { .dl_state = SDO_SRV_DL_START };
	struct can_frame frame_in = { 0 }, frame_out = { 0 };
	
	sdo_set_cs(&frame_in, SDO_CCS_DL_INIT_REQ);
	ASSERT_INT_EQ(SDO_SRV_DL_ABORT, sdo_srv_dl_sm_seg(&sm, &frame_in,
								&frame_out, 0));
	ASSERT_INT_EQ(SDO_SRV_DL_ABORT, sm.dl_state);
	ASSERT_INT_EQ(SDO_SCS_DL_ABORT, sdo_get_cs(&frame_out));
	ASSERT_INT_EQ(SDO_ABORT_INVALID_CS, sdo_get_abort_code(&frame_out));

	return 0;
}

int sdo_srv_dl_seg_remote_abort()
{
	struct sdo_srv_dl_sm sm = { .dl_state = SDO_SRV_DL_START };
	struct can_frame frame_in = { 0 }, frame_out = { 0 };
	
	sdo_set_cs(&frame_in, SDO_CCS_DL_ABORT);
	ASSERT_INT_EQ(SDO_SRV_DL_REMOTE_ABORT,
		      sdo_srv_dl_sm_seg(&sm, &frame_in, &frame_out, 0));
	ASSERT_INT_EQ(SDO_SRV_DL_REMOTE_ABORT, sm.dl_state);

	return 0;
}

int sdo_srv_dl_example()
{
	struct sdo_srv_dl_sm sm = { .dl_state = SDO_SRV_DL_START };
	struct can_frame frame_in = { 0 };
	struct can_frame frame_out = { 0 };

	sdo_set_cs(&frame_in, SDO_CCS_DL_INIT_REQ);
	strcpy((char*)&frame_in.data[SDO_MULTIPLEXER_IDX], "ab");

	ASSERT_INT_EQ(SDO_SRV_DL_SEG, sdo_srv_dl_sm_feed(&sm, &frame_in,
							 &frame_out));
	ASSERT_INT_EQ(SDO_SCS_DL_INIT_RES, sdo_get_cs(&frame_out));
	ASSERT_STR_EQ("ab", (char*)&frame_out.data[SDO_MULTIPLEXER_IDX]);

	sdo_set_cs(&frame_in, SDO_CCS_DL_SEG_REQ);
	ASSERT_INT_EQ(SDO_SRV_DL_SEG_TOGGLED, sdo_srv_dl_sm_feed(&sm, &frame_in,
								 &frame_out));
	ASSERT_INT_EQ(SDO_SCS_DL_SEG_RES, sdo_get_cs(&frame_out));
	ASSERT_FALSE(sdo_is_toggled(&frame_out));
	ASSERT_FALSE(sdo_is_end_segment(&frame_out));

	sdo_set_cs(&frame_in, SDO_CCS_DL_SEG_REQ);
	sdo_toggle(&frame_in);
	ASSERT_INT_EQ(SDO_SRV_DL_SEG, sdo_srv_dl_sm_feed(&sm, &frame_in,
							 &frame_out));
	ASSERT_INT_EQ(SDO_SCS_DL_SEG_RES, sdo_get_cs(&frame_out));
	ASSERT_TRUE(sdo_is_toggled(&frame_out));
	ASSERT_FALSE(sdo_is_end_segment(&frame_out));

	sdo_set_cs(&frame_in, SDO_CCS_DL_SEG_REQ);
	sdo_toggle(&frame_in);
	ASSERT_INT_EQ(SDO_SRV_DL_SEG_TOGGLED, sdo_srv_dl_sm_feed(&sm, &frame_in,
								 &frame_out));
	ASSERT_INT_EQ(SDO_SCS_DL_SEG_RES, sdo_get_cs(&frame_out));
	ASSERT_FALSE(sdo_is_toggled(&frame_out));
	ASSERT_FALSE(sdo_is_end_segment(&frame_out));

	sdo_set_cs(&frame_in, SDO_CCS_DL_SEG_REQ);
	sdo_toggle(&frame_in);
	sdo_end_segment(&frame_in);
	ASSERT_INT_EQ(SDO_SRV_DL_DONE, sdo_srv_dl_sm_feed(&sm, &frame_in,
							  &frame_out));
	ASSERT_INT_EQ(SDO_SCS_DL_SEG_RES, sdo_get_cs(&frame_out));
	ASSERT_TRUE(sdo_is_toggled(&frame_out));
	ASSERT_TRUE(sdo_is_end_segment(&frame_out));
	
	return 0;
}


int main()
{
	int r = 0;
	RUN_TEST(set_get_cs);
	RUN_TEST(set_get_segment_size);
	RUN_TEST(set_get_abort_code);
	RUN_TEST(sdo_srv_dl_init_ok);
	RUN_TEST(sdo_srv_dl_init_failed_cs);
	RUN_TEST(sdo_srv_dl_init_remote_abort);
	RUN_TEST(sdo_srv_dl_seg_ok);
	RUN_TEST(sdo_srv_dl_seg_toggled_ok);
	RUN_TEST(sdo_srv_dl_seg_end_ok);
	RUN_TEST(sdo_srv_dl_seg_toggled_end_ok);
	RUN_TEST(sdo_srv_dl_seg_failed_cs);
	RUN_TEST(sdo_srv_dl_seg_remote_abort);
	RUN_TEST(sdo_srv_dl_example);
	return r;
}


