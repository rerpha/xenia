/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2017 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_GPU_REGISTERS_H_
#define XENIA_GPU_REGISTERS_H_

#include <cstdint>
#include <cstdlib>

#include "xenia/base/assert.h"
#include "xenia/gpu/xenos.h"

// Most registers can be found from:
// https://github.com/UDOOboard/Kernel_Unico/blob/master/drivers/mxc/amd-gpu/include/reg/yamato/14/yamato_registers.h
// Some registers were added on Adreno specifically and are not referenced in
// game .pdb files and never set by games.

// Only 32-bit types (uint32_t, int32_t, float or enums with uint32_t / int32_t
// as the underlying type) are allowed in the bit fields here, as Visual C++
// restarts packing when a field requires different alignment than the previous
// one.

namespace xe {
namespace gpu {

enum Register {
#define XE_GPU_REGISTER(index, type, name) XE_GPU_REG_##name = index,
#include "xenia/gpu/register_table.inc"
#undef XE_GPU_REGISTER
};

namespace reg {

/*******************************************************************************
   ___ ___  _  _ _____ ___  ___  _
  / __/ _ \| \| |_   _| _ \/ _ \| |
 | (_| (_) | .` | | | |   / (_) | |__
  \___\___/|_|\_| |_| |_|_\\___/|____|

*******************************************************************************/

union alignas(uint32_t) COHER_STATUS_HOST {
  uint32_t value;
  struct {
    uint32_t matching_contexts : 8;      // +0
    uint32_t rb_copy_dest_base_ena : 1;  // +8
    uint32_t dest_base_0_ena : 1;        // +9
    uint32_t dest_base_1_ena : 1;        // +10
    uint32_t dest_base_2_ena : 1;        // +11
    uint32_t dest_base_3_ena : 1;        // +12
    uint32_t dest_base_4_ena : 1;        // +13
    uint32_t dest_base_5_ena : 1;        // +14
    uint32_t dest_base_6_ena : 1;        // +15
    uint32_t dest_base_7_ena : 1;        // +16
    uint32_t : 7;                        // +17
    uint32_t vc_action_ena : 1;          // +24
    uint32_t tc_action_ena : 1;          // +25
    uint32_t pglb_action_ena : 1;        // +26
    uint32_t : 4;                        // +27
    uint32_t status : 1;                 // +31
  };
  static constexpr Register register_index = XE_GPU_REG_COHER_STATUS_HOST;
};
static_assert_size(COHER_STATUS_HOST, sizeof(uint32_t));

union alignas(uint32_t) WAIT_UNTIL {
  uint32_t value;
  struct {
    uint32_t : 1;                    // +0
    uint32_t wait_re_vsync : 1;      // +1
    uint32_t wait_fe_vsync : 1;      // +2
    uint32_t wait_vsync : 1;         // +3
    uint32_t wait_dsply_id0 : 1;     // +4
    uint32_t wait_dsply_id1 : 1;     // +5
    uint32_t wait_dsply_id2 : 1;     // +6
    uint32_t : 3;                    // +7
    uint32_t wait_cmdfifo : 1;       // +10
    uint32_t : 3;                    // +11
    uint32_t wait_2d_idle : 1;       // +14
    uint32_t wait_3d_idle : 1;       // +15
    uint32_t wait_2d_idleclean : 1;  // +16
    uint32_t wait_3d_idleclean : 1;  // +17
    uint32_t : 2;                    // +18
    uint32_t cmdfifo_entries : 4;    // +20
  };
  static constexpr Register register_index = XE_GPU_REG_WAIT_UNTIL;
};
static_assert_size(WAIT_UNTIL, sizeof(uint32_t));

/*******************************************************************************
  ___ ___ ___  _   _ ___ _  _  ___ ___ ___
 / __| __/ _ \| | | | __| \| |/ __| __| _ \
 \__ \ _| (_) | |_| | _|| .` | (__| _||   /
 |___/___\__\_\\___/|___|_|\_|\___|___|_|_\

*******************************************************************************/

union alignas(uint32_t) SQ_PROGRAM_CNTL {
  uint32_t value;
  struct {
    // Note from a2xx.xml:
    // Only 0x3F worth of valid register values for VS_NUM_REG and PS_NUM_REG,
    // but high bit is set to indicate "0 registers used".
    // (Register count = (num_reg & 0x80) ? 0 : (num_reg + 1))
    uint32_t vs_num_reg : 8;                           // +0
    uint32_t ps_num_reg : 8;                           // +8
    uint32_t vs_resource : 1;                          // +16
    uint32_t ps_resource : 1;                          // +17
    uint32_t param_gen : 1;                            // +18
    uint32_t gen_index_pix : 1;                        // +19
    uint32_t vs_export_count : 4;                      // +20
    xenos::VertexShaderExportMode vs_export_mode : 3;  // +24
    uint32_t ps_export_mode : 4;                       // +27
    uint32_t gen_index_vtx : 1;                        // +31
  };
  static constexpr Register register_index = XE_GPU_REG_SQ_PROGRAM_CNTL;
};
static_assert_size(SQ_PROGRAM_CNTL, sizeof(uint32_t));

union alignas(uint32_t) SQ_CONTEXT_MISC {
  uint32_t value;
  struct {
    uint32_t inst_pred_optimize : 1;          // +0
    uint32_t sc_output_screen_xy : 1;         // +1
    xenos::SampleControl sc_sample_cntl : 2;  // +2
    uint32_t : 4;                             // +4
    // Pixel shader interpolator (according to the XNA microcode compiler) index
    // to write pixel parameters to. So far have been able to find the following
    // usage:
    // * |XY| - position on screen (vPos - the XNA microcode compiler translates
    //   ps_3_0 vPos directly to this, so at least in Direct3D 9 pixel center
    //   mode, this contains 0, 1, 2, not 0.5, 1.5, 2.5). flto also said in the
    //   Freedreno IRC that it's .0 even in OpenGL:
    //   https://dri.freedesktop.org/~cbrill/dri-log/?channel=freedreno&date=2020-04-19
    //   (on Android, according to LG P705 GL_OES_get_program_binary
    //   disassembly, gl_FragCoord.xy is |r0.xy| * c221.xy + c222.zw - haven't
    //   been able to dump the constant values by exploiting a huge uniform
    //   array, but flto says c222.zw contains tile offset plus 0.5).
    // * Sign bit of X - is front face (vFace), non-negative for front face,
    //   negative for back face (used with `rcpc` in shaders to take signedness
    //   of 0 into account in `cndge`).
    // * |ZW| - UV within a point sprite (sign meaning is unknown so far).
    uint32_t param_gen_pos : 8;    // +8
    uint32_t perfcounter_ref : 1;  // +16
    uint32_t yeild_optimize : 1;   // +17 sic
    uint32_t tx_cache_sel : 1;     // +18
  };
  static constexpr Register register_index = XE_GPU_REG_SQ_CONTEXT_MISC;
};
static_assert_size(SQ_CONTEXT_MISC, sizeof(uint32_t));

union alignas(uint32_t) SQ_INTERPOLATOR_CNTL {
  uint32_t value;
  struct {
    uint32_t param_shade : 16;  // +0
    // SampleLocation bits - 0 for centroid, 1 for center, if
    // SQ_CONTEXT_MISC::sc_sample_cntl is kCentroidsAndCenters.
    uint32_t sampling_pattern : 16;  // +16
  };
  static constexpr Register register_index = XE_GPU_REG_SQ_INTERPOLATOR_CNTL;
};
static_assert_size(SQ_INTERPOLATOR_CNTL, sizeof(uint32_t));

/*******************************************************************************
 __   _____ ___ _____ _____  __
 \ \ / / __| _ \_   _| __\ \/ /
  \ V /| _||   / | | | _| >  <
   \_/ |___|_|_\ |_| |___/_/\_\

   ___ ___  ___  _   _ ___ ___ ___     _   _  _ ___
  / __| _ \/ _ \| | | | _ \ __| _ \   /_\ | \| |   \
 | (_ |   / (_) | |_| |  _/ _||   /  / _ \| .` | |) |
  \___|_|_\\___/ \___/|_| |___|_|_\ /_/ \_\_|\_|___/

  _____ ___ ___ ___ ___ _    _      _ _____ ___  ___
 |_   _| __/ __/ __| __| |  | |    /_\_   _/ _ \| _ \
   | | | _|\__ \__ \ _|| |__| |__ / _ \| || (_) |   /
   |_| |___|___/___/___|____|____/_/ \_\_| \___/|_|_\

*******************************************************************************/

union alignas(uint32_t) VGT_DMA_SIZE {
  uint32_t value;
  struct {
    uint32_t num_words : 24;      // +0
    uint32_t : 6;                 // +24
    xenos::Endian swap_mode : 2;  // +30
  };
  static constexpr Register register_index = XE_GPU_REG_VGT_DMA_SIZE;
};

union alignas(uint32_t) VGT_DRAW_INITIATOR {
  uint32_t value;
  // Different than on A2xx and R6xx/R7xx.
  struct {
    xenos::PrimitiveType prim_type : 6;     // +0
    xenos::SourceSelect source_select : 2;  // +6
    xenos::MajorMode major_mode : 2;        // +8
    uint32_t : 1;                           // +10
    xenos::IndexFormat index_size : 1;      // +11
    uint32_t not_eop : 1;                   // +12
    uint32_t : 3;                           // +13
    uint32_t num_indices : 16;              // +16
  };
  static constexpr Register register_index = XE_GPU_REG_VGT_DRAW_INITIATOR;
};
static_assert_size(VGT_DRAW_INITIATOR, sizeof(uint32_t));

// Unlike on R6xx (but closer to R5xx), and according to the Adreno 200 header,
// the registers related to the vertex index are 24-bit. Vertex indices are
// unsigned, and only the lower 24 bits of them are actually used by the GPU -
// this has been verified on an Adreno 200 phone (LG Optimus L7) on OpenGL ES
// using a GL_UNSIGNED_INT element array buffer with junk in the upper 8 bits
// that had no effect on drawing.

// The order of operations is primitive reset index checking -> offsetting ->
// clamping.

union alignas(uint32_t) VGT_MULTI_PRIM_IB_RESET_INDX {
  uint32_t value;
  struct {
    // The upper 8 bits of the value from the index buffer are confirmed to be
    // ignored. So, though this specifically is untested (because
    // GL_PRIMITIVE_RESTART_FIXED_INDEX was added only in OpenGL ES 3.0, though
    // it behaves conceptually close to our expectations anyway - uses the
    // 0xFFFFFFFF restart index while GL_MAX_ELEMENT_INDEX may be 0xFFFFFF),
    // the restart index check likely only involves the lower 24 bit of the
    // vertex index - therefore, if reset_indx is 0xFFFFFF, likely 0xFFFFFF,
    // 0x1FFFFFF, 0xFFFFFFFF all cause primitive reset.
    uint32_t reset_indx : 24;
  };
  static constexpr Register register_index =
      XE_GPU_REG_VGT_MULTI_PRIM_IB_RESET_INDX;
};
static_assert_size(VGT_MULTI_PRIM_IB_RESET_INDX, sizeof(uint32_t));

union alignas(uint32_t) VGT_INDX_OFFSET {
  uint32_t value;
  struct {
    // Unlike R5xx's VAP_INDEX_OFFSET, which is signed 25-bit, this is 24-bit -
    // and signedness doesn't matter as index calculations are done in 24-bit
    // integers, and ((0xFFFFFE + 3) & 0xFFFFFF) == 1 anyway, just like
    // ((0xFFFFFFFE + 3) & 0xFFFFFF) == 1 if we treated it as signed by
    // sign-extending on the host. Direct3D 9 just writes BaseVertexIndex as a
    // signed int32 to the entire register, but the upper 8 bits are ignored
    // anyway, and that has no effect on offsets that fit in 24 bits.
    uint32_t indx_offset : 24;
  };
  static constexpr Register register_index = XE_GPU_REG_VGT_INDX_OFFSET;
};
static_assert_size(VGT_INDX_OFFSET, sizeof(uint32_t));

union alignas(uint32_t) VGT_MIN_VTX_INDX {
  uint32_t value;
  struct {
    uint32_t min_indx : 24;
  };
  static constexpr Register register_index = XE_GPU_REG_VGT_MIN_VTX_INDX;
};
static_assert_size(VGT_MIN_VTX_INDX, sizeof(uint32_t));

union alignas(uint32_t) VGT_MAX_VTX_INDX {
  uint32_t value;
  struct {
    // Usually 0xFFFF or 0xFFFFFF.
    uint32_t max_indx : 24;
  };
  static constexpr Register register_index = XE_GPU_REG_VGT_MAX_VTX_INDX;
};
static_assert_size(VGT_MAX_VTX_INDX, sizeof(uint32_t));

union alignas(uint32_t) VGT_OUTPUT_PATH_CNTL {
  uint32_t value;
  struct {
    xenos::VGTOutputPath path_select : 2;  // +0
  };
  static constexpr Register register_index = XE_GPU_REG_VGT_OUTPUT_PATH_CNTL;
};
static_assert_size(VGT_OUTPUT_PATH_CNTL, sizeof(uint32_t));

union alignas(uint32_t) VGT_HOS_CNTL {
  uint32_t value;
  struct {
    xenos::TessellationMode tess_mode : 2;  // +0
  };
  static constexpr Register register_index = XE_GPU_REG_VGT_HOS_CNTL;
};
static_assert_size(VGT_HOS_CNTL, sizeof(uint32_t));

/*******************************************************************************
  ___ ___ ___ __  __ ___ _____ _____   _____
 | _ \ _ \_ _|  \/  |_ _|_   _|_ _\ \ / / __|
 |  _/   /| || |\/| || |  | |  | | \ V /| _|
 |_| |_|_\___|_|  |_|___| |_| |___| \_/ |___|

    _   ___ ___ ___ __  __ ___ _    ___ ___
   /_\ / __/ __| __|  \/  | _ ) |  | __| _ \
  / _ \\__ \__ \ _|| |\/| | _ \ |__| _||   /
 /_/ \_\___/___/___|_|  |_|___/____|___|_|_\

*******************************************************************************/

union alignas(uint32_t) PA_SU_POINT_MINMAX {
  uint32_t value;
  struct {
    // Radius, 12.4 fixed point.
    uint32_t min_size : 16;  // +0
    uint32_t max_size : 16;  // +16
  };
  static constexpr Register register_index = XE_GPU_REG_PA_SU_POINT_MINMAX;
};
static_assert_size(PA_SU_POINT_MINMAX, sizeof(uint32_t));

union alignas(uint32_t) PA_SU_POINT_SIZE {
  uint32_t value;
  struct {
    // 1/2 width or height, 12.4 fixed point.
    uint32_t height : 16;  // +0
    uint32_t width : 16;   // +16
  };
  static constexpr Register register_index = XE_GPU_REG_PA_SU_POINT_SIZE;
};
static_assert_size(PA_SU_POINT_SIZE, sizeof(uint32_t));

// Setup Unit / Scanline Converter mode cntl
union alignas(uint32_t) PA_SU_SC_MODE_CNTL {
  uint32_t value;
  struct {
    uint32_t cull_front : 1;  // +0
    uint32_t cull_back : 1;   // +1
    // 0 - front is CCW, 1 - front is CW.
    uint32_t face : 1;  // +2
    // 4541096E uses poly_mode 2 for triangles, which is "reserved" on R6xx and
    // not defined on Adreno 2xx, but polymode_front/back_ptype are 0 (points)
    // in this case in 4541096E, which should not be respected for non-kDualMode
    // as the title wants to draw filled triangles.
    xenos::PolygonModeEnable poly_mode : 2;       // +3
    xenos::PolygonType polymode_front_ptype : 3;  // +5
    xenos::PolygonType polymode_back_ptype : 3;   // +8
    uint32_t poly_offset_front_enable : 1;        // +11
    uint32_t poly_offset_back_enable : 1;         // +12
    uint32_t poly_offset_para_enable : 1;         // +13
    uint32_t : 1;                                 // +14
    uint32_t msaa_enable : 1;                     // +15
    uint32_t vtx_window_offset_enable : 1;        // +16
    // LINE_STIPPLE_ENABLE was added on Adreno.
    uint32_t : 2;                        // +17
    uint32_t provoking_vtx_last : 1;     // +19
    uint32_t persp_corr_dis : 1;         // +20
    uint32_t multi_prim_ib_ena : 1;      // +21
    uint32_t : 1;                        // +22
    uint32_t quad_order_enable : 1;      // +23
    uint32_t sc_one_quad_per_clock : 1;  // +24
    // WAIT_RB_IDLE_ALL_TRI and WAIT_RB_IDLE_FIRST_TRI_NEW_STATE were added on
    // Adreno.
  };
  static constexpr Register register_index = XE_GPU_REG_PA_SU_SC_MODE_CNTL;
};
static_assert_size(PA_SU_SC_MODE_CNTL, sizeof(uint32_t));

// Setup Unit Vertex Control
union alignas(uint32_t) PA_SU_VTX_CNTL {
  uint32_t value;
  struct {
    uint32_t pix_center : 1;  // +0 1 = half pixel offset (OpenGL).
    uint32_t round_mode : 2;  // +1
    uint32_t quant_mode : 3;  // +3
  };
  static constexpr Register register_index = XE_GPU_REG_PA_SU_VTX_CNTL;
};
static_assert_size(PA_SU_VTX_CNTL, sizeof(uint32_t));

union alignas(uint32_t) PA_SC_MPASS_PS_CNTL {
  uint32_t value;
  struct {
    uint32_t mpass_pix_vec_per_pass : 20;  // +0
    uint32_t : 11;                         // +20
    uint32_t mpass_ps_ena : 1;             // +31
  };
  static constexpr Register register_index = XE_GPU_REG_PA_SC_MPASS_PS_CNTL;
};
static_assert_size(PA_SC_MPASS_PS_CNTL, sizeof(uint32_t));

// Scanline converter viz query, used by D3D for gpu side conditional rendering
union alignas(uint32_t) PA_SC_VIZ_QUERY {
  uint32_t value;
  struct {
    // the visibility of draws should be evaluated
    uint32_t viz_query_ena : 1;  // +0
    uint32_t viz_query_id : 6;   // +1
    // discard geometry after test (but use for testing)
    uint32_t kill_pix_post_hi_z : 1;  // +7
    // not used with d3d
    uint32_t kill_pix_post_detail_mask : 1;  // +8
  };
  static constexpr Register register_index = XE_GPU_REG_PA_SC_VIZ_QUERY;
};
static_assert_size(PA_SC_VIZ_QUERY, sizeof(uint32_t));

// Clipper clip control
union alignas(uint32_t) PA_CL_CLIP_CNTL {
  uint32_t value;
  struct {
    uint32_t ucp_ena_0 : 1;               // +0
    uint32_t ucp_ena_1 : 1;               // +1
    uint32_t ucp_ena_2 : 1;               // +2
    uint32_t ucp_ena_3 : 1;               // +3
    uint32_t ucp_ena_4 : 1;               // +4
    uint32_t ucp_ena_5 : 1;               // +5
    uint32_t : 8;                         // +6
    uint32_t ps_ucp_mode : 2;             // +14
    uint32_t clip_disable : 1;            // +16
    uint32_t ucp_cull_only_ena : 1;       // +17
    uint32_t boundary_edge_flag_ena : 1;  // +18
    uint32_t dx_clip_space_def : 1;       // +19
    uint32_t dis_clip_err_detect : 1;     // +20
    uint32_t vtx_kill_or : 1;             // +21
    uint32_t xy_nan_retain : 1;           // +22
    uint32_t z_nan_retain : 1;            // +23
    uint32_t w_nan_retain : 1;            // +24
  };
  static constexpr Register register_index = XE_GPU_REG_PA_CL_CLIP_CNTL;
};
static_assert_size(PA_CL_CLIP_CNTL, sizeof(uint32_t));

// Viewport transform engine control
union alignas(uint32_t) PA_CL_VTE_CNTL {
  uint32_t value;
  struct {
    uint32_t vport_x_scale_ena : 1;   // +0
    uint32_t vport_x_offset_ena : 1;  // +1
    uint32_t vport_y_scale_ena : 1;   // +2
    uint32_t vport_y_offset_ena : 1;  // +3
    uint32_t vport_z_scale_ena : 1;   // +4
    uint32_t vport_z_offset_ena : 1;  // +5
    uint32_t : 2;                     // +6
    uint32_t vtx_xy_fmt : 1;          // +8
    uint32_t vtx_z_fmt : 1;           // +9
    uint32_t vtx_w0_fmt : 1;          // +10
    uint32_t perfcounter_ref : 1;     // +11
  };
  static constexpr Register register_index = XE_GPU_REG_PA_CL_VTE_CNTL;
};
static_assert_size(PA_CL_VTE_CNTL, sizeof(uint32_t));

union alignas(uint32_t) PA_SC_SCREEN_SCISSOR_TL {
  uint32_t value;
  struct {
    int32_t tl_x : 15;  // +0
    uint32_t : 1;       // +15
    int32_t tl_y : 15;  // +16
  };
  static constexpr Register register_index = XE_GPU_REG_PA_SC_SCREEN_SCISSOR_TL;
};
static_assert_size(PA_SC_SCREEN_SCISSOR_TL, sizeof(uint32_t));

union alignas(uint32_t) PA_SC_SCREEN_SCISSOR_BR {
  uint32_t value;
  struct {
    int32_t br_x : 15;  // +0
    uint32_t : 1;       // +15
    int32_t br_y : 15;  // +16
  };
  static constexpr Register register_index = XE_GPU_REG_PA_SC_SCREEN_SCISSOR_BR;
};
static_assert_size(PA_SC_SCREEN_SCISSOR_BR, sizeof(uint32_t));

union alignas(uint32_t) PA_SC_WINDOW_OFFSET {
  uint32_t value;
  struct {
    int32_t window_x_offset : 15;  // +0
    uint32_t : 1;                  // +15
    int32_t window_y_offset : 15;  // +16
  };
  static constexpr Register register_index = XE_GPU_REG_PA_SC_WINDOW_OFFSET;
};
static_assert_size(PA_SC_WINDOW_OFFSET, sizeof(uint32_t));

union alignas(uint32_t) PA_SC_WINDOW_SCISSOR_TL {
  uint32_t value;
  struct {
    uint32_t tl_x : 14;                  // +0
    uint32_t : 2;                        // +14
    uint32_t tl_y : 14;                  // +16
    uint32_t : 1;                        // +30
    uint32_t window_offset_disable : 1;  // +31
  };
  static constexpr Register register_index = XE_GPU_REG_PA_SC_WINDOW_SCISSOR_TL;
};
static_assert_size(PA_SC_WINDOW_SCISSOR_TL, sizeof(uint32_t));

union alignas(uint32_t) PA_SC_WINDOW_SCISSOR_BR {
  uint32_t value;
  struct {
    uint32_t br_x : 14;  // +0
    uint32_t : 2;        // +14
    uint32_t br_y : 14;  // +16
  };
  static constexpr Register register_index = XE_GPU_REG_PA_SC_WINDOW_SCISSOR_BR;
};
static_assert_size(PA_SC_WINDOW_SCISSOR_BR, sizeof(uint32_t));

/*******************************************************************************
  ___ ___ _  _ ___  ___ ___
 | _ \ __| \| |   \| __| _ \
 |   / _|| .` | |) | _||   /
 |_|_\___|_|\_|___/|___|_|_\

  ___   _   ___ _  _____ _  _ ___
 | _ ) /_\ / __| |/ / __| \| |   \
 | _ \/ _ \ (__| ' <| _|| .` | |) |
 |___/_/ \_\___|_|\_\___|_|\_|___/

*******************************************************************************/

union alignas(uint32_t) RB_MODECONTROL {
  uint32_t value;
  struct {
    xenos::ModeControl edram_mode : 3;  // +0
  };
  static constexpr Register register_index = XE_GPU_REG_RB_MODECONTROL;
};
static_assert_size(RB_MODECONTROL, sizeof(uint32_t));

union alignas(uint32_t) RB_SURFACE_INFO {
  uint32_t value;
  struct {
    uint32_t surface_pitch : 14;          // +0 in pixels.
    uint32_t : 2;                         // +14
    xenos::MsaaSamples msaa_samples : 2;  // +16
    uint32_t hiz_pitch : 14;              // +18
  };
  static constexpr Register register_index = XE_GPU_REG_RB_SURFACE_INFO;
};
static_assert_size(RB_SURFACE_INFO, sizeof(uint32_t));

union alignas(uint32_t) RB_COLORCONTROL {
  uint32_t value;
  struct {
    xenos::CompareFunction alpha_func : 3;  // +0
    uint32_t alpha_test_enable : 1;         // +3
    uint32_t alpha_to_mask_enable : 1;      // +4
    // Everything in between was added on Adreno.
    uint32_t : 19;  // +5
    // According to tests on an Adreno 200 device (LG Optimus L7), done by
    // drawing 0.5x0.5 rectangles in different corners of four pixels in a quad
    // to a multisampled GLSurfaceView, the coverage mask is the following for 4
    // samples:
    // 0.25)  [0.25, 0.5)  [0.5, 0.75)  [0.75, 1)   [1
    //  --        --           --          --       --
    // |  |      |  |         | #|        |##|     |##|
    // |  |      |# |         |# |        |# |     |##|
    //  --        --           --          --       --
    // (gl_FragCoord.y near 0 in the top, near 1 in the bottom here - D3D-like.)
    // For 2 samples, the top sample (closer to gl_FragCoord.y 0) is covered
    // when alpha is in [0.5, 1), the bottom sample is covered when the alpha is
    // [1. With these thresholds, however, in 5454082B, almost all distant trees
    // are transparent, this is asymmetric - fully transparent for a quarter of
    // the range (or even half of the range for 2x and almost the entire range
    // for 1x), but fully opaque only in one value.
    // Though, 2, 2, 2, 2 offset values are commonly used for undithered alpha
    // to coverage (in games such as 5454082B, and overall in AMD driver
    // implementations) - it appears that 2, 2, 2, 2 offsets are supposed to
    // make this symmetric.
    // Both 5454082B and RADV (which used AMDVLK as a reference) use 3, 1, 0, 2
    // offsets for dithered alpha to mask.
    // https://gitlab.freedesktop.org/nchery/mesa/commit/8a52e4cc4fad4f1c75acc0badd624778f9dfe202
    // It appears that the offsets lower the thresholds by (offset / 4 /
    // sample count). That's consistent with both 2, 2, 2, 2 making the test
    // symmetric and 0, 0, 0, 0 (forgetting to set the offset values) resulting
    // in what the official Adreno 200 driver for Android (which is pretty buggy
    // overall) produces.
    // According to Evergreen register reference:
    // - offset0 is for pixel (0, 0) in each quad.
    // - offset1 is for pixel (0, 1) in each quad.
    // - offset2 is for pixel (1, 0) in each quad.
    // - offset3 is for pixel (1, 1) in each quad.
    uint32_t alpha_to_mask_offset0 : 2;  // +24
    uint32_t alpha_to_mask_offset1 : 2;  // +26
    uint32_t alpha_to_mask_offset2 : 2;  // +28
    uint32_t alpha_to_mask_offset3 : 2;  // +30
  };
  static constexpr Register register_index = XE_GPU_REG_RB_COLORCONTROL;
};
static_assert_size(RB_COLORCONTROL, sizeof(uint32_t));

union alignas(uint32_t) RB_COLOR_INFO {
  uint32_t value;
  struct {
    uint32_t color_base : 12;                         // +0 in tiles.
    uint32_t : 4;                                     // +12
    xenos::ColorRenderTargetFormat color_format : 4;  // +16
    int32_t color_exp_bias : 6;                       // +20
  };
  static constexpr Register register_index = XE_GPU_REG_RB_COLOR_INFO;
  // RB_COLOR[1-3]_INFO also use this format.
  static const Register rt_register_indices[4];
};
static_assert_size(RB_COLOR_INFO, sizeof(uint32_t));

union alignas(uint32_t) RB_COLOR_MASK {
  uint32_t value;
  struct {
    uint32_t write_red0 : 1;    // +0
    uint32_t write_green0 : 1;  // +1
    uint32_t write_blue0 : 1;   // +2
    uint32_t write_alpha0 : 1;  // +3
    uint32_t write_red1 : 1;    // +4
    uint32_t write_green1 : 1;  // +5
    uint32_t write_blue1 : 1;   // +6
    uint32_t write_alpha1 : 1;  // +7
    uint32_t write_red2 : 1;    // +8
    uint32_t write_green2 : 1;  // +9
    uint32_t write_blue2 : 1;   // +10
    uint32_t write_alpha2 : 1;  // +11
    uint32_t write_red3 : 1;    // +12
    uint32_t write_green3 : 1;  // +13
    uint32_t write_blue3 : 1;   // +14
    uint32_t write_alpha3 : 1;  // +15
  };
  static constexpr Register register_index = XE_GPU_REG_RB_COLOR_MASK;
};
static_assert_size(RB_COLOR_MASK, sizeof(uint32_t));

union alignas(uint32_t) RB_BLENDCONTROL {
  uint32_t value;
  struct {
    xenos::BlendFactor color_srcblend : 5;   // +0
    xenos::BlendOp color_comb_fcn : 3;       // +5
    xenos::BlendFactor color_destblend : 5;  // +8
    uint32_t : 3;                            // +13
    xenos::BlendFactor alpha_srcblend : 5;   // +16
    xenos::BlendOp alpha_comb_fcn : 3;       // +21
    xenos::BlendFactor alpha_destblend : 5;  // +24
    // BLEND_FORCE_ENABLE and BLEND_FORCE were added on Adreno.
  };
  // RB_BLENDCONTROL[0-3] use this format.
  static constexpr Register register_index = XE_GPU_REG_RB_BLENDCONTROL0;
  static const Register rt_register_indices[4];
};
static_assert_size(RB_BLENDCONTROL, sizeof(uint32_t));

union alignas(uint32_t) RB_DEPTHCONTROL {
  uint32_t value;
  struct {
    uint32_t stencil_enable : 1;  // +0
    uint32_t z_enable : 1;        // +1
    uint32_t z_write_enable : 1;  // +2
    // EARLY_Z_ENABLE was added on Adreno.
    uint32_t : 1;                               // +3
    xenos::CompareFunction zfunc : 3;           // +4
    uint32_t backface_enable : 1;               // +7
    xenos::CompareFunction stencilfunc : 3;     // +8
    xenos::StencilOp stencilfail : 3;           // +11
    xenos::StencilOp stencilzpass : 3;          // +14
    xenos::StencilOp stencilzfail : 3;          // +17
    xenos::CompareFunction stencilfunc_bf : 3;  // +20
    xenos::StencilOp stencilfail_bf : 3;        // +23
    xenos::StencilOp stencilzpass_bf : 3;       // +26
    xenos::StencilOp stencilzfail_bf : 3;       // +29
  };
  static constexpr Register register_index = XE_GPU_REG_RB_DEPTHCONTROL;
};
static_assert_size(RB_DEPTHCONTROL, sizeof(uint32_t));

union alignas(uint32_t) RB_STENCILREFMASK {
  uint32_t value;
  struct {
    uint32_t stencilref : 8;        // +0
    uint32_t stencilmask : 8;       // +8
    uint32_t stencilwritemask : 8;  // +16
  };
  static constexpr Register register_index = XE_GPU_REG_RB_STENCILREFMASK;
  // RB_STENCILREFMASK_BF also uses this format.
};
static_assert_size(RB_STENCILREFMASK, sizeof(uint32_t));

union alignas(uint32_t) RB_DEPTH_INFO {
  uint32_t value;
  struct {
    uint32_t depth_base : 12;                         // +0 in tiles.
    uint32_t : 4;                                     // +12
    xenos::DepthRenderTargetFormat depth_format : 1;  // +16
  };
  static constexpr Register register_index = XE_GPU_REG_RB_DEPTH_INFO;
};
static_assert_size(RB_DEPTH_INFO, sizeof(uint32_t));

// Copy registers are very different than on Adreno.

union alignas(uint32_t) RB_COPY_CONTROL {
  uint32_t value;
  struct {
    uint32_t copy_src_select : 3;                    // +0 Depth is 4.
    uint32_t : 1;                                    // +3
    xenos::CopySampleSelect copy_sample_select : 3;  // +4
    uint32_t : 1;                                    // +7
    uint32_t color_clear_enable : 1;                 // +8
    uint32_t depth_clear_enable : 1;                 // +9
    uint32_t : 10;                                   // +10
    xenos::CopyCommand copy_command : 2;             // +20
  };
  static constexpr Register register_index = XE_GPU_REG_RB_COPY_CONTROL;
};
static_assert_size(RB_COPY_CONTROL, sizeof(uint32_t));

union alignas(uint32_t) RB_COPY_DEST_INFO {
  uint32_t value;
  struct {
    xenos::Endian128 copy_dest_endian : 3;            // +0
    uint32_t copy_dest_array : 1;                     // +3
    uint32_t copy_dest_slice : 3;                     // +4
    xenos::ColorFormat copy_dest_format : 6;          // +7
    xenos::SurfaceNumberFormat copy_dest_number : 3;  // +13
    int32_t copy_dest_exp_bias : 6;                   // +16
    uint32_t : 2;                                     // +22
    uint32_t copy_dest_swap : 1;                      // +24
  };
  static constexpr Register register_index = XE_GPU_REG_RB_COPY_DEST_INFO;
};
static_assert_size(RB_COPY_DEST_INFO, sizeof(uint32_t));

union alignas(uint32_t) RB_COPY_DEST_PITCH {
  uint32_t value;
  struct {
    uint32_t copy_dest_pitch : 14;   // +0
    uint32_t : 2;                    // +14
    uint32_t copy_dest_height : 14;  // +16
  };
  static constexpr Register register_index = XE_GPU_REG_RB_COPY_DEST_PITCH;
};
static_assert_size(RB_COPY_DEST_PITCH, sizeof(uint32_t));

}  // namespace reg

}  // namespace gpu
}  // namespace xe

#endif  // XENIA_GPU_REGISTERS_H_
