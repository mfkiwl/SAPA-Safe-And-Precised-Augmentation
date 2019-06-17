/*
 Write and read SAPA HPAC format to and from buffer
*/
#include <stdio.h>
//#include "platform.h"

#include "gpp_sapa.h"
#include "bit2buff.h"



static const GPPDOUBLE SAPA_IONO_QUALITY[16] = { 0.00, 0.03, 0.05,0.07, 0.14, 0.28, 0.56,1.12,2.24,4.48,8.96,17.92,35.84,71.68,143.36,143.36 };
static const GPPDOUBLE SAPA_TROPO_QUALITY[8] = { 0.0,0.010,0.020,0.040,0.080,0.160,0.320,0.320 };

static const GPPUINT1 SAPA_TROPO_RESIDUAL_ZENITH_DELAY[2] = { SAPA_SMALL_TROPO_RESIDUAL_ZENITH_DELAY, SAPA_LARGE_TROPO_RESIDUAL_ZENITH_DELAY };
static const GPPUINT1 SAPA_IONO_RESIDUAL_SLANT_DELAY[4] = { SAPA_SMALL_IONO_RESIDUAL_SLANT_DELAY,SAPA_MEDIUM_IONO_RESIDUAL_SLANT_DELAY,SAPA_LARGE_IONO_RESIDUAL_SLANT_DELAY ,SAPA_EXTRA_LARGE_IONO_RESIDUAL_SLANT_DELAY };

const GPPUINT1 SAPA_TROPO_COEFF_T012[2] = { 7,9 };
const GPPUINT1 SAPA_TROPO_COEFF_T3[2] = { 9,11};
const GPPUINT1 *SAPA_TROPO_COEFF[4] = { SAPA_TROPO_COEFF_T012,SAPA_TROPO_COEFF_T012,SAPA_TROPO_COEFF_T012 ,SAPA_TROPO_COEFF_T3 };

const GPPUINT1 SAPA_IONO_COEFF_C012[2] = { 12,14 };
const GPPUINT1 SAPA_IONO_COEFF_C3[2] = { 13,15 };
const GPPUINT1 *SAPA_IONO_COEFF[4] = { SAPA_IONO_COEFF_C012,SAPA_IONO_COEFF_C012,SAPA_IONO_COEFF_C012 ,SAPA_IONO_COEFF_C3 };


//------------------------------------------------------Declaration of functions to store data in Buffer for HPAC ----------------------------------------------------
static GPPLONG gpp_sapa_hpac_header2buffer(const pGPP_SAPA_HPAC p_hpac, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_atmo2buffer(const pGPP_SAPA_HPAC p_hpac, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_area2buffer(const pGPP_SAPA_HPAC p_hpac, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_tropo2buffer(const pGPP_SAPA_HPAC p_hpac,  GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_tropo_poly_coefficient_block2buffer(const pGPP_SAPA_HPAC p_hpac, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_tropo_grid_block2buffer(const pGPP_SAPA_HPAC p_hpac, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_iono2buffer(const pGPP_SAPA_HPAC p_hpac, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_iono_sv_bitmask2buffer(GPPUINT8 sv_prn_bits, GPPUINT1 sys, GPPUINT1 *svlist, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_iono_sat2buffer(const pGPP_SAPA_HPAC p_hpac, GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_iono_sat_poly2buffer(const pGPP_SAPA_HPAC p_hpac, GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_iono_sat_coeff2buffer(const pGPP_SAPA_HPAC p_hpac, GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_iono_sat_grid2buffer(const pGPP_SAPA_HPAC p_hpac, GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);

//-----------------------------------------------------Delecation of function to read data from buffer for HPAC------------------------------------------------------------------------
static GPPLONG gpp_sapa_hpac_buffer2header(pGPP_SAPA_HPAC p_hpac, const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_buffer2atmo(pGPP_SAPA_HPAC p_hpac, GPPUINT1 area, const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_buffer2area(pGPP_SAPA_HPAC p_hpac, GPPUINT1 area, const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_buffer2tropo(pGPP_SAPA_HPAC p_hpac, GPPUINT1 area, const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_buffer2tropo_poly_coefficient_block(pGPP_SAPA_HPAC p_hpac, GPPUINT1 area, const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_buffer2tropo_grid_block(pGPP_SAPA_HPAC p_hpac, GPPUINT1 area, const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_buffer2iono(pGPP_SAPA_HPAC p_hpac, GPPUINT1 area, const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_buffer2iono_sv_bitmask(GPPUINT8 *sv_prn_bits, GPPUINT1 sys, GPPUINT1 *svlist, const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_buffer2iono_sat(pGPP_SAPA_HPAC p_hpac, GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area, const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_buffer2iono_sat_poly(pGPP_SAPA_HPAC p_hpac, GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area, const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_buffer2iono_sat_coeff(pGPP_SAPA_HPAC p_hpac, GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area, const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
static GPPLONG gpp_sapa_hpac_buffer2iono_sat_grid(pGPP_SAPA_HPAC p_hpac, GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area, const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos);
/**************************************************************************
 *	\brief Write SAPA HPAC message to buffer
 *
 *	\param[in] 	hpac 	Pointer to GPP_SAPA_HPAC structure
 *	\param[in]	sys		GNSS indicator
 *	\param[in]	*buffer
 *	\param[in]	*byte_pos
 *	\param[in]	*bit_pos
 *
 *	\retval 	Number of needed bits
 *	\retval 	Error Code
 *************************************************************************/
GPPLONG gpp_sapa_hpac2buffer(const pGPP_SAPA_HPAC hpac, GPPUINT1 sys,GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG rc;
	GPPUINT1 ia,no_of_areas;

	GPPLONG byte_pos0, bit_pos0;
	GPPLONG mybyte = 0, mybit = 0;

	// handling if byte_pos or bit_pos are null pointers
	if (!byte_pos) 	byte_pos = &mybyte;
	if (!bit_pos) 	bit_pos = &mybit;

	//printf(" byte: %d bit: %d \n", *byte_pos, *bit_pos);
	// get start position of byte_pos and bit_pos to get needed bits
	byte_pos0 = *byte_pos;
	bit_pos0 = *bit_pos;

	if (rc = gpp_sapa_hpac_header2buffer(hpac,buffer, byte_pos, bit_pos)) return rc;
	//printf(" byte: %d bit: %d \n", *byte_pos, *bit_pos);
	no_of_areas = hpac->header_block->area_count;
	for (ia = 0; ia < no_of_areas; ia++) {
		if (rc = gpp_sapa_hpac_atmo2buffer(hpac, ia, buffer, byte_pos, bit_pos)) return rc;
	}
	return  gpp_sapa_get_bit_diff(*byte_pos, *bit_pos, byte_pos0, bit_pos0);
}//gpp_sapa_hpac2buffer()


/**************************************************************************
 *	\brief Read SAPA HPAC message from buffer
 *
 *	\param[in] 	hpac 	Pointer to GPP_SAPA_OCB structure
 *	\param[in]	*buffer
 *	\param[in]	*byte_pos
 *	\param[in]	*bit_pos
 *
 *	\retval 	Number of read bits
 *	\retval 	Error Code
 *************************************************************************/
GPPLONG gpp_sapa_buffer2hpac(pGPP_SAPA_HPAC hpac, const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG rc;
	GPPLONG byte_pos0, bit_pos0;
	GPPLONG mybyte = 0, mybit = 0;

	GPPUINT1 ia, no_of_areas;

	// handling if byte_pos or bit_pos are null pointers
	if (!byte_pos) 	byte_pos = &mybyte;
	if (!bit_pos) 	bit_pos = &mybit;

	// get start position of byte_pos and bit_pos to get needed bits
	byte_pos0 = *byte_pos;
	bit_pos0 = *bit_pos;

	if (rc = gpp_sapa_hpac_buffer2header(hpac,buffer, byte_pos, bit_pos)) return rc;
	no_of_areas = hpac->header_block->area_count;
	for (ia = 0; ia < no_of_areas; ia++) {
		if (rc = gpp_sapa_hpac_buffer2atmo(hpac, ia, buffer, byte_pos, bit_pos)) return rc;
	}
	return  gpp_sapa_get_bit_diff(*byte_pos, *bit_pos, byte_pos0, bit_pos0);
}//gpp_sapa_buffer2hpac()
/******************************************************************************
 *	\brief Write HPAC header to buffer
 *
 *	\param[in]	hpac
 *	\param[in]	*buffer
 *	\param[in]	*byte_pos
 *	\param[in]	*bit_pos
 *
 *	\retval 0 Ok
 *	\retval Error Code
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_header2buffer(const pGPP_SAPA_HPAC p_hpac,GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPUINT1 time_tag_type;
	/*printf("ok");
	printf("value=%d", p_hpac->header_block->message_sub_type);
	printf("value of byte pos=%d", byte_pos);
	printf("value of bit pos=%d", bit_pos);*/

	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 4, p_hpac->header_block->message_sub_type);										//SF001

	time_tag_type = p_hpac->header_block->time_tag_type;

	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 1, time_tag_type);											//SF002

	if (time_tag_type == GNSS_FULL_TIME_TAG){
		gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 32, p_hpac->header_block->time_tag);											//SF004
	}
	else if (time_tag_type == GNSS_HOURLY_TIME_TAG){
		gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 12, p_hpac->header_block->time_tag);											//SF003
	}

	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 7, p_hpac->header_block->sol_id);													//SF006
	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 4, p_hpac->header_block->sol_processor_id);										//SF007
	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 9, p_hpac->header_block->sol_issue_of_update);									    //SF005
	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 1, p_hpac->header_block->reserved);
	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 4, p_hpac->header_block->area_issue_of_update);
	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 5, p_hpac->header_block->area_count);												//SF030
	return 0;
}//gpp_sapa_hpac_header2buffer()
/******************************************************************************
 *	\brief Read HPAC header from buffer
 *
 *	\param[in]	hpac
 *	\param[in]	sys
 *	\param[in]	*buffer
 *	\param[in]	*byte_pos
 *	\param[in]	*bit_pos
 *
 *	\retval 0 Ok
 *	\retval Error Code
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_buffer2header(pGPP_SAPA_HPAC p_hpac,const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG rc;
	GPP_SAPA_HPAC_HEADER header = { 0, };
	header.message_sub_type=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 4);										//SF001
	header.time_tag_type=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 1);										//SF002
	if (header.time_tag_type == GNSS_FULL_TIME_TAG)
	{
		header.time_tag=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 32);														//SF004
	}
	else if (header.time_tag_type == GNSS_HOURLY_TIME_TAG)
	{
		header.time_tag=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 12);														//SF003
	}
	header.sol_id=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 7);												//SF006
	header.sol_processor_id=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 4);										//SF007
	header.sol_issue_of_update=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 9);									//SF005
	header.reserved = gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 1);									//SF069
	header.area_issue_of_update = gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 4);									//SF068
	header.area_count=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 5);											//SF030
	if(rc=gpp_sapa_hpac_add_header(p_hpac, &header)) return rc;
	return 0;
}//gpp_sapa_hpac_buffer2header()
/******************************************************************************
 *	\brief Store atmosphere block data into buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_atmo2buffer(const pGPP_SAPA_HPAC p_hpac,GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG rc;
	if(rc=gpp_sapa_hpac_area2buffer(p_hpac,area,buffer, byte_pos, bit_pos)) return rc;
	if(rc=gpp_sapa_hpac_tropo2buffer(p_hpac, area, buffer, byte_pos, bit_pos)) return rc;
	if(rc=gpp_sapa_hpac_iono2buffer(p_hpac, area, buffer, byte_pos, bit_pos)) return rc;
	return 0;
}//gpp_sapa_hpac_atmo2buffer()
/******************************************************************************
 *	\brief Read atmosphere block data from buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_buffer2atmo(pGPP_SAPA_HPAC p_hpac, GPPUINT1 area,const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG rc;
	if (rc = gpp_sapa_hpac_buffer2area(p_hpac,area, buffer, byte_pos, bit_pos)) return rc;
	if (rc=gpp_sapa_hpac_buffer2tropo(p_hpac, area, buffer, byte_pos, bit_pos)) return rc;
	if (rc=gpp_sapa_hpac_buffer2iono(p_hpac, area, buffer, byte_pos, bit_pos)) return rc;
	return 0;
}//gpp_sapa_hpac_buffer2atmo()
/******************************************************************************
 *	\brief Store Area block data into buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_area2buffer(const pGPP_SAPA_HPAC p_hpac,GPPUINT1 area,GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 8, p_hpac->atmo[area]->area_def->area_id);																		//SF031
	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 7, p_hpac->atmo[area]->area_def->number_of_grid_point);															//SF039
	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 2, p_hpac->atmo[area]->area_def->tropo_block_indicator);														//SF040
	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 2, p_hpac->atmo[area]->area_def->iono_block_indicator);															//SF040
    return 0;
}//gpp_sapa_hpac_area2buffer()
/******************************************************************************
 *	\brief Read Area block data from buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_buffer2area(pGPP_SAPA_HPAC p_hpac, GPPUINT1 area,const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG rc;
	GPP_SAPA_HPAC_AREA area_def = { 0, };
	area_def.area_id = gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 8);																		//SF031																															//SF015
	area_def.number_of_grid_point=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 7);															//SF039
	area_def.tropo_block_indicator=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 2);														//SF040
	area_def.iono_block_indicator=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 2);															//SF040
	if (rc = gpp_sapa_hpac_add_area(p_hpac, area, &area_def)) return rc;
	return 0;
}//gpp_sapa_hpac_buffer2area()
/******************************************************************************
 *	\brief Store Troposphere block data into buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_tropo2buffer(const pGPP_SAPA_HPAC p_hpac, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG  rc=0;
	if (p_hpac->atmo[area]->area_def->tropo_block_indicator !=0)					//1 or 2
	{
		if(rc=gpp_sapa_hpac_tropo_poly_coefficient_block2buffer(p_hpac, area,buffer, byte_pos, bit_pos)) return 0;
	}
	if (p_hpac->atmo[area]->area_def->tropo_block_indicator == 2)
		if(rc=gpp_sapa_hpac_tropo_grid_block2buffer(p_hpac, area,buffer, byte_pos, bit_pos)) return 0;
	return rc;
}//gpp_sapa_hpac_tropo2buffer()
/******************************************************************************
 *	\brief Read Troposphere block data from buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_buffer2tropo(pGPP_SAPA_HPAC p_hpac,GPPUINT1 area,const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG  rc=0;
	if (p_hpac->atmo[area]->area_def->tropo_block_indicator != 0)					//1 or 2
	{
		if (rc = gpp_sapa_hpac_buffer2tropo_poly_coefficient_block(p_hpac, area, buffer, byte_pos, bit_pos)) return 0;
	}
	if (p_hpac->atmo[area]->area_def->tropo_block_indicator == 2)
		if (rc = gpp_sapa_hpac_buffer2tropo_grid_block(p_hpac, area, buffer, byte_pos, bit_pos)) return 0;
	return rc;
}//gpp_sapa_hpac_buffer2tropo()

/******************************************************************************
 *	\brief Store Troposphere Polynomial Coefficient block data into buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_tropo_poly_coefficient_block2buffer(const pGPP_SAPA_HPAC p_hpac,GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPUINT1 coeff_size, equ_type;
	GPPUINT1 bit_id = 1;
	pGPP_SAPA_HPAC_TROPO_POLY_COEFFICIENT_BLOCK tpcb = NULL;

	coeff_size = p_hpac->atmo[area]->tropo->tropo_poly_coeff_block->tropo_coeff_size;
	equ_type = p_hpac->atmo[area]->tropo->tropo_poly_coeff_block->tropo_equation_type;
	tpcb = p_hpac->atmo[area]->tropo->tropo_poly_coeff_block;

	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 3, equ_type);											//SF041

	while (bit_id <= 7 && p_hpac->atmo[area]->tropo->tropo_poly_coeff_block->tropo_quality >= SAPA_TROPO_QUALITY[bit_id]) bit_id++;
	bit_id -= 1;

	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 3, bit_id);																									//SF042
	gn_add_val_double_to_buffer(buffer, byte_pos, bit_pos, 8, p_hpac->atmo[area]->tropo->tropo_poly_coeff_block->tropo_avhd, SAPA_RES_AREA_AVG_VER_HEDRO_DELAY);			//SF043
	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 1, p_hpac->atmo[area]->tropo->tropo_poly_coeff_block->tropo_coeff_size);											//SF044
	gn_add_val_double_to_buffer(buffer, byte_pos, bit_pos, SAPA_TROPO_COEFF[coeff_size][TROPO_POLY_COEFF_INDX_T00], tpcb->tropo_poly_coeff[TROPO_POLY_COEFF_INDX_T00], SAPA_RES_TROPO_POLY_COEFF_T00);	//SF045orSF048

	if (equ_type!=0){
		gn_add_val_double_to_buffer(buffer, byte_pos, bit_pos, SAPA_TROPO_COEFF[coeff_size][TROPO_POLY_COEFF_INDX_T01], tpcb->tropo_poly_coeff[TROPO_POLY_COEFF_INDX_T01], SAPA_RES_TROPO_POLY_COEFF_T01);	//SF046orSF049
		gn_add_val_double_to_buffer(buffer, byte_pos, bit_pos, SAPA_TROPO_COEFF[coeff_size][TROPO_POLY_COEFF_INDX_T10], tpcb->tropo_poly_coeff[TROPO_POLY_COEFF_INDX_T10], SAPA_RES_TROPO_POLY_COEFF_T01);	//SF046orSF049
	}
	if (equ_type == 2){
		gn_add_val_double_to_buffer(buffer, byte_pos, bit_pos, SAPA_TROPO_COEFF[coeff_size][TROPO_POLY_COEFF_INDX_T11], tpcb->tropo_poly_coeff[TROPO_POLY_COEFF_INDX_T11], SAPA_RES_TROPO_POLY_COEFF_T11);	//SF047orSF050
	}
	return 0;
}//gpp_sapa_hpac_tropo_poly_coefficient_block2buffer()
/******************************************************************************
 *	\brief Read Troposphere Polynomial Coefficient block data from buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_buffer2tropo_poly_coefficient_block(pGPP_SAPA_HPAC p_hpac, GPPUINT1 area,const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG rc;
	GPPUINT1 coeff_size;
	GPP_SAPA_HPAC_TROPO_POLY_COEFFICIENT_BLOCK tropo_poly_coeff_block = { 0, };

	tropo_poly_coeff_block.tropo_equation_type = gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 3);										//SF041
	tropo_poly_coeff_block.tropo_quality = SAPA_TROPO_QUALITY[gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 3)];																							//SF042
	tropo_poly_coeff_block.tropo_avhd = gn_get_val_double_from_buffer(buffer, byte_pos, bit_pos, 8, SAPA_RES_AREA_AVG_VER_HEDRO_DELAY);			//SF043
	tropo_poly_coeff_block.tropo_coeff_size = gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 1);											//SF044

	coeff_size = tropo_poly_coeff_block.tropo_coeff_size;

	tropo_poly_coeff_block.tropo_poly_coeff[TROPO_POLY_COEFF_INDX_T00]=gn_get_val_double_from_buffer(buffer, byte_pos, bit_pos, SAPA_TROPO_COEFF[coeff_size][TROPO_POLY_COEFF_INDX_T00], SAPA_RES_TROPO_POLY_COEFF_T00);	//SF045orSF048
	if (tropo_poly_coeff_block.tropo_equation_type != 0){
		tropo_poly_coeff_block.tropo_poly_coeff[TROPO_POLY_COEFF_INDX_T01]=gn_get_val_double_from_buffer(buffer, byte_pos, bit_pos, SAPA_TROPO_COEFF[coeff_size][TROPO_POLY_COEFF_INDX_T01], SAPA_RES_TROPO_POLY_COEFF_T01);	//SF046orSF049
		tropo_poly_coeff_block.tropo_poly_coeff[TROPO_POLY_COEFF_INDX_T10]= gn_get_val_double_from_buffer(buffer, byte_pos, bit_pos, SAPA_TROPO_COEFF[coeff_size][TROPO_POLY_COEFF_INDX_T10], SAPA_RES_TROPO_POLY_COEFF_T01);	//SF046orSF049
	}
	if (tropo_poly_coeff_block.tropo_equation_type == 2){
		tropo_poly_coeff_block.tropo_poly_coeff[TROPO_POLY_COEFF_INDX_T11]= gn_get_val_double_from_buffer(buffer, byte_pos, bit_pos, SAPA_TROPO_COEFF[coeff_size][TROPO_POLY_COEFF_INDX_T11], SAPA_RES_TROPO_POLY_COEFF_T11);	//SF047orSF050
	}
	if (rc = gpp_sapa_hpac_add_tropo_poly_coeff_block(p_hpac, area, &tropo_poly_coeff_block)) return rc;
	return 0;
}//gpp_sapa_hpac_buffer2tropo_poly_coefficient_block()
/******************************************************************************
 *	\brief Store Troposphere Grid block data into buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_tropo_grid_block2buffer(const pGPP_SAPA_HPAC p_hpac,GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPUINT1 no_of_grid, ig;
	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 1, p_hpac->atmo[area]->tropo->tropo_grid->tropo_residual_size); //SF051
	no_of_grid= p_hpac->atmo[area]->area_def->number_of_grid_point;
	for (ig = 0; ig < no_of_grid; ig++)
	{
		gn_add_val_double_to_buffer(buffer, byte_pos, bit_pos, SAPA_TROPO_RESIDUAL_ZENITH_DELAY[p_hpac->atmo[area]->tropo->tropo_grid->tropo_residual_size], p_hpac->atmo[area]->tropo->tropo_grid->tropo_residuals[ig], SAPA_RES_TROPO_RESIDUAL_DELAY);//SF052
	}
	return 0;
}//gpp_sapa_hpac_tropo_grid_block2buffer()
/******************************************************************************
 *	\brief Read Troposphere Grid block data from buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_buffer2tropo_grid_block(pGPP_SAPA_HPAC p_hpac, GPPUINT1 area,const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG rc;
	GPP_SAPA_HPAC_TROPO_GRID_BLOCK tropo_grid = { 0, };
	GPPUINT1 no_of_grid, ig;
	tropo_grid.tropo_residual_size=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 1); //SF051
	no_of_grid = p_hpac->atmo[area]->area_def->number_of_grid_point;
	for (ig = 0; ig < no_of_grid; ig++)
	{
		tropo_grid.tropo_residuals[ig]=gn_get_val_double_from_buffer(buffer, byte_pos, bit_pos, SAPA_TROPO_RESIDUAL_ZENITH_DELAY[tropo_grid.tropo_residual_size], SAPA_RES_TROPO_RESIDUAL_DELAY);//SF052
	}
	if(rc=gpp_sapa_hpac_add_tropo_grid_block(p_hpac,area, &tropo_grid)) return rc;
	return 0;
}//gpp_sapa_hpac_buffer2tropo_grid_block()
/******************************************************************************
 *	\brief Store Ionospher block data into buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_iono2buffer(const pGPP_SAPA_HPAC p_hpac, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPUINT1 svlist[66] = { 0, };
	if (p_hpac->atmo[area]->area_def->iono_block_indicator != 0)
	{
		GPPLONG rc;
		GPPUINT1 isat,sys;
		//printf("hi");
		gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 3, p_hpac->atmo[area]->iono->iono_equation_type);     //SF054
		//printf("hello");
		for (sys = 0; sys < 2; sys++)
		{
			if (rc = gpp_sapa_hpac_iono_sv_bitmask2buffer(p_hpac->atmo[area]->iono->sat_prn_bits[sys], sys, svlist, buffer, byte_pos, bit_pos)) return rc;
			for (isat = 1; isat <= svlist[0]; isat++) {
				GPPUINT1 sat;
				sat = svlist[isat];
				if (rc = gpp_sapa_hpac_iono_sat2buffer(p_hpac, sys, sat, area, buffer, byte_pos, bit_pos)) return rc;
			}
		}
	}
	return 0;
}//gpp_sapa_hpac_iono2buffer()
/******************************************************************************
 *	\brief Read Ionospher block data from buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_buffer2iono(pGPP_SAPA_HPAC p_hpac, GPPUINT1 area,const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPP_SAPA_HPAC_IONO iono = { 0, };
	GPPUINT1 svlist[66] = { 0, };

	if (p_hpac->atmo[area]->area_def->iono_block_indicator != 0)
	{
		GPPLONG rc;
		GPPUINT1 isat,sys;
		iono.iono_equation_type=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 3);     //SF054
		if (rc = gpp_sapa_hpac_add_iono(p_hpac,area, &iono)) return rc;

		for (sys = 0; sys < 2; sys++)
		{
			if (rc = gpp_sapa_hpac_buffer2iono_sv_bitmask(&iono.sat_prn_bits[sys], sys, svlist, buffer, byte_pos, bit_pos)) return rc;
			for (isat = 1; isat <= svlist[0]; isat++) {
				GPPUINT1 sat;
				sat = svlist[isat];
				if (rc = gpp_sapa_hpac_buffer2iono_sat(p_hpac, sys, sat, area, buffer, byte_pos, bit_pos)) return rc;
			}
		}
	}
	return 0;
}//gpp_sapa_hpac_buffer2iono()
/******************************************************************************
 *	\brief Store Ionospher  Bit Mask  data into buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_iono_sv_bitmask2buffer(GPPUINT8 sv_prn_bits, GPPUINT1 sys, GPPUINT1 *svlist, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPUINT1 bit_id = 0;

	if (!svlist) return GPP_SAPA_ERR_INVALID_SVLIST;

	gpp_sapa_get_svlist(sv_prn_bits, svlist);
	bit_id = 0;
	while (svlist[0] - SAPA_SV_BITMASK_LEN[sys][bit_id] > 0 && bit_id <= 3) bit_id++;							//Check (> || <)

	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 2, bit_id);																	//SF011
	gn_add_longlong_to_buffer(buffer, byte_pos, bit_pos, SAPA_SV_BITMASK_LEN[sys][bit_id], sv_prn_bits);

	return 0;
}//gpp_sapa_hpac_iono_sv_bitmask2buffer()
/******************************************************************************
 *	\brief Read Ionospher  Bit Mask  data from buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_buffer2iono_sv_bitmask(GPPUINT8 *sv_prn_bits, GPPUINT1 sys, GPPUINT1 *svlist, const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPUINT1 bit_id;

	bit_id = gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 2);
	*sv_prn_bits= gn_get_ulonglong_from_buffer(buffer, byte_pos, bit_pos, SAPA_SV_BITMASK_LEN[sys][bit_id]);
	gpp_sapa_get_svlist(*sv_prn_bits, svlist);

	return 0;
}//gpp_sapa_hpac_buffer2iono_sv_bitmask()
/******************************************************************************
 *	\brief Store Ionospher Satellite block data into buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_iono_sat2buffer(const pGPP_SAPA_HPAC p_hpac,GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG rc=0;
	if(rc=gpp_sapa_hpac_iono_sat_poly2buffer(p_hpac, sys,sat, area, buffer, byte_pos, bit_pos)) return 0;
	if(rc=gpp_sapa_hpac_iono_sat_coeff2buffer(p_hpac,sys, sat, area, buffer, byte_pos, bit_pos)) return 0;
	if(rc=gpp_sapa_hpac_iono_sat_grid2buffer(p_hpac, sys,sat, area, buffer, byte_pos, bit_pos)) return 0;
	return rc;
}//gpp_sapa_hpac_iono_sat2buffer()
/******************************************************************************
 *	\brief Read Ionospher Satellite block data from buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_buffer2iono_sat(pGPP_SAPA_HPAC p_hpac, GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area,const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG rc=0;
	if (rc = gpp_sapa_hpac_buffer2iono_sat_poly(p_hpac, sys, sat, area, buffer, byte_pos, bit_pos)) return 0;
	if (rc = gpp_sapa_hpac_buffer2iono_sat_coeff(p_hpac, sys, sat, area, buffer, byte_pos, bit_pos)) return 0;
	if (rc = gpp_sapa_hpac_buffer2iono_sat_grid(p_hpac, sys, sat,area, buffer, byte_pos, bit_pos)) return 0;
	return rc;
}//gpp_sapa_hpac_buffer2iono_sat()
/******************************************************************************
 *	\brief Store Ionospher Satellite Polynomial block data into buffer
 ******************************************************************************/

static GPPLONG gpp_sapa_hpac_iono_sat_poly2buffer(const pGPP_SAPA_HPAC p_hpac, GPPUINT1 sys,GPPUINT1 sat, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPUINT1 bit_id=1;
	while (bit_id <= 15 && p_hpac->atmo[area]->iono->iono_sat_block[sys][sat]->iono_sat_poly->iono_quality >= SAPA_IONO_QUALITY[bit_id]) bit_id++;
	bit_id -= 1;
	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 4, bit_id);			//SF055
	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos, 1, p_hpac->atmo[area]->iono->iono_sat_block[sys][sat]->iono_sat_poly->iono_coeff_size);					//SF056
	return 0;
}//gpp_sapa_hpac_iono_sat_poly2buffer()
/******************************************************************************
 *	\brief Read Ionospher Satellite Polynomial block data from buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_buffer2iono_sat_poly(pGPP_SAPA_HPAC p_hpac, GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area,const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG rc;
	GPP_SAPA_HPAC_IONO_SAT_POLY iono_sat_poly = { 0, };
	iono_sat_poly.iono_quality=SAPA_IONO_QUALITY[gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 4)];				//SF055
	iono_sat_poly.iono_coeff_size=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 1);								//SF056
	if(rc=gpp_sapa_hpac_add_iono_sat_poly_block(p_hpac,sys,sat,area,&iono_sat_poly)) return rc;
	return 0;
}//gpp_sapa_hpac_buffer2iono_sat_poly()
/******************************************************************************
 *	\brief Store Ionospher Satellite Coefficient  block data into buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_iono_sat_coeff2buffer(const pGPP_SAPA_HPAC p_hpac, GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPUINT1 coeff_size, equ_type;
	pGPP_SAPA_HPAC_IONO_SAT_COEFFICIENT tpcb = NULL;

	coeff_size = p_hpac->atmo[area]->iono->iono_sat_block[sys][sat]->iono_sat_poly->iono_coeff_size;
	equ_type = p_hpac->atmo[area]->iono->iono_equation_type;

	tpcb = p_hpac->atmo[area]->iono->iono_sat_block[sys][sat]->iono_sat_coeff;
	gn_add_val_double_to_buffer(buffer, byte_pos, bit_pos, SAPA_IONO_COEFF[coeff_size][IONO_POLY_COEFF_INDX_C00], tpcb->iono_poly_coeff[IONO_POLY_COEFF_INDX_C00], SAPA_RES_IONO_POLY_COEFF_C00);	//SF057orSF060
	if (equ_type!=0){
		gn_add_val_double_to_buffer(buffer, byte_pos, bit_pos, SAPA_IONO_COEFF[coeff_size][IONO_POLY_COEFF_INDX_C01], tpcb->iono_poly_coeff[IONO_POLY_COEFF_INDX_C01], SAPA_RES_IONO_POLY_COEFF_C01);	//SF058orSF061
		gn_add_val_double_to_buffer(buffer, byte_pos, bit_pos, SAPA_IONO_COEFF[coeff_size][IONO_POLY_COEFF_INDX_C10], tpcb->iono_poly_coeff[IONO_POLY_COEFF_INDX_C10], SAPA_RES_IONO_POLY_COEFF_C01);	//SF058orSF061
	}

	if (equ_type == 2){
		gn_add_val_double_to_buffer(buffer, byte_pos, bit_pos, SAPA_IONO_COEFF[coeff_size][IONO_POLY_COEFF_INDX_C11], tpcb->iono_poly_coeff[IONO_POLY_COEFF_INDX_C11], SAPA_RES_IONO_POLY_COEFF_C11);	//SF059orSF062
	}
	return 0;
}//gpp_sapa_hpac_iono_sat_coeff2buffer()
/******************************************************************************
 *	\brief Read Ionospher Satellite Coefficient  block data from buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_buffer2iono_sat_coeff(pGPP_SAPA_HPAC p_hpac,GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area,const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG rc;
	GPPUINT1 coeff_size, equ_type;
	GPP_SAPA_HPAC_IONO_SAT_COEFFICIENT	iono_sat_coeff = { 0, };

	coeff_size = p_hpac->atmo[area]->iono->iono_sat_block[sys][sat]->iono_sat_poly->iono_coeff_size;
	equ_type = p_hpac->atmo[area]->iono->iono_equation_type;

	iono_sat_coeff.iono_poly_coeff[IONO_POLY_COEFF_INDX_C00]=gn_get_val_double_from_buffer(buffer, byte_pos, bit_pos, SAPA_IONO_COEFF[coeff_size][IONO_POLY_COEFF_INDX_C00], SAPA_RES_IONO_POLY_COEFF_C00);	//SF057orSF060

	if(equ_type!=0){
		iono_sat_coeff.iono_poly_coeff[IONO_POLY_COEFF_INDX_C01]=gn_get_val_double_from_buffer(buffer, byte_pos, bit_pos, SAPA_IONO_COEFF[coeff_size][IONO_POLY_COEFF_INDX_C01], SAPA_RES_IONO_POLY_COEFF_C01);	//SF058orSF061
		iono_sat_coeff.iono_poly_coeff[IONO_POLY_COEFF_INDX_C10]=gn_get_val_double_from_buffer(buffer, byte_pos, bit_pos, SAPA_IONO_COEFF[coeff_size][IONO_POLY_COEFF_INDX_C10], SAPA_RES_IONO_POLY_COEFF_C01);	//SF058orSF061
	}

	if(equ_type==2){
		iono_sat_coeff.iono_poly_coeff[IONO_POLY_COEFF_INDX_C11]=gn_get_val_double_from_buffer(buffer, byte_pos, bit_pos, SAPA_IONO_COEFF[coeff_size][IONO_POLY_COEFF_INDX_C11], SAPA_RES_IONO_POLY_COEFF_C11);	//SF059orSF062
	}

	if(rc=gpp_sapa_hpac_add_iono_sat_coeff_block(p_hpac,sys,sat,area,&iono_sat_coeff)) return rc;

	return 0;
}//gpp_sapa_hpac_buffer2iono_sat_coeff()
/******************************************************************************
 *	\brief Store Ionospher Satellite Grid block data into buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_iono_sat_grid2buffer(const pGPP_SAPA_HPAC p_hpac,GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area, GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPUINT1 no_of_grid,ig;
	gn_add_ulong_to_buffer(buffer, byte_pos, bit_pos,2, p_hpac->atmo[area]->iono->iono_sat_block[sys][sat]->iono_grid->iono_residual_field_size);	//SF063
	no_of_grid = p_hpac->atmo[area]->area_def->number_of_grid_point;
	for (ig = 0; ig < no_of_grid; ig++)
	{
		gn_add_val_double_to_buffer(buffer, byte_pos, bit_pos, SAPA_IONO_RESIDUAL_SLANT_DELAY[p_hpac->atmo[area]->iono->iono_sat_block[sys][sat]->iono_grid->iono_residual_field_size], p_hpac->atmo[area]->iono->iono_sat_block[sys][sat]->iono_grid->iono_residuals[ig], SAPA_RES_IONO_RESIDUAL_DELAY);//SF064
	}
	return 0;
}//gpp_sapa_hpac_iono_sat_grid2buffer()
/******************************************************************************
 *	\brief Read Ionospher Satellite Grid block data from buffer
 ******************************************************************************/
static GPPLONG gpp_sapa_hpac_buffer2iono_sat_grid(pGPP_SAPA_HPAC p_hpac,GPPUINT1 sys, GPPUINT1 sat, GPPUINT1 area,const GPPUCHAR *buffer, GPPLONG *byte_pos, GPPLONG *bit_pos)
{
	GPPLONG rc;
	GPP_SAPA_HPAC_IONO_GRID_BLOCK iono_grid = { 0, };
	GPPUINT1 no_of_grid, ig;
	iono_grid.iono_residual_field_size=gn_get_ulong_from_buffer(buffer, byte_pos, bit_pos, 2);	//SF063
	no_of_grid = p_hpac->atmo[area]->area_def->number_of_grid_point;
	for (ig = 0; ig < no_of_grid; ig++)
	{
		iono_grid.iono_residuals[ig]=gn_get_val_double_from_buffer(buffer, byte_pos, bit_pos, SAPA_IONO_RESIDUAL_SLANT_DELAY[iono_grid.iono_residual_field_size],SAPA_RES_IONO_RESIDUAL_DELAY);//SF064
	}
	if (rc = gpp_sapa_hpac_add_iono_sat_grid_block(p_hpac, sys, sat, area, &iono_grid)) return rc;
	return 0;
}//gpp_sapa_hpac_buffer2iono_sat_grid()