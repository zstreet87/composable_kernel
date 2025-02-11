#include <stdlib.h>
#include "config.hpp"
#include "device_conv2d_backward_weight_xdl_c_shuffle_nhwc_kyxc_nhwk.hpp"
#include "element_wise_operation.hpp"
#include "device_operation_instance.hpp"

namespace ck {
namespace tensor_operation {
namespace device {
namespace device_conv2d_bwd_weight_instance {

using F16 = ck::half_t;
using F32 = float;

template <ck::index_t... Is>
using S = ck::Sequence<Is...>;

using PassThrough = ck::tensor_operation::element_wise::PassThrough;

// Compilation parameters for in[n, hi, wi, c] * wei[k, y, x, c] = out[n, ho, wo, k]
using device_conv2d_bwd_weight_xdl_nhwc_kyxc_nhwk_f16_instances = std::tuple<
    // clang-format off
        //#################################################################################| InData| WeiData| OutData| AccData|          In|         Wei|         Out|  Block|  MPer|  NPer| K0Per| K1| MPer| NPer| MXdl| NXdl|  ABlockTransfer|   ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockLds|   BBlockTransfer|  BBlockTransfer| BBlockTransfer| BlockTransfer| BBlockTransfer| BBlockTransfer| BBlockLds|    CShuffle|    CShuffle|    CBlockTransfer| CBlockTransfer|
        //#################################################################################|   Type|    Type|    Type|    Type| Elementwise| Elementwise| Elementwise|   Size| Block| Block| Block|   |  XDL|  XDL|  Per|  Per|   ThreadCluster|    ThreadCluster| SrcAccessOrder|   SrcVectorDim|      SrcScalar|      DstScalar| AddExtraM|    ThreadCluster|   ThreadCluster| SrcAccessOrder|  SrcVectorDim|      SrcScalar|      DstScalar| AddExtraN| MXdlPerWave| NXdlPerWave|    ClusterLengths|ScalarPerVector|
        //#################################################################################|       |        |        |        |   Operation|   Operation|   Operation|       |      |      |      |   |     |     | Wave| Wave| Lengths_K0_M_K1|     ArrangeOrder|               |               |      PerVector|   PerVector_K1|          |  Lengths_K0_N_K1|    ArrangeOrder|               |              |      PerVector|   PerVector_K1|          |  PerShuffle|  PerShuffle|  MBlock_MPerBlock|   NWaveNPerXdl|
        //#################################################################################|       |        |        |        |            |            |            |       |      |      |      |   |     |     |     |     |                |                 |               |               |               |               |          |                 |                |               |              |               |               |          |            |            |  NBlock_NPerBlock|               |
        DeviceConv2dBwdWeightXdl_C_Shuffle_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<    F16,     F16,     F16,     F32,  PassThrough, PassThrough, PassThrough,    256,   256,   128,    4,  8,   32,   32,    4,    2,  S<1, 4, 32, 2>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              4,    true,     S<1, 4, 16, 4>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              2,      true,        1,            1,     S<1, 32, 1, 8>,        8>,
        DeviceConv2dBwdWeightXdl_C_Shuffle_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<    F16,     F16,     F16,     F32,  PassThrough, PassThrough, PassThrough,    256,   128,   256,    4,  8,   32,   32,    2,    4,  S<1, 4, 16, 4>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              2,    true,     S<1, 4, 32, 2>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              4,      true,        1,            1,     S<1, 32, 1, 8>,        8>,
        DeviceConv2dBwdWeightXdl_C_Shuffle_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<    F16,     F16,     F16,     F32,  PassThrough, PassThrough, PassThrough,    128,   128,   128,    4,  8,   32,   32,    4,    2,  S<1, 4, 16, 2>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              4,    true,     S<1, 4, 16, 2>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              4,      true,        1,            1,     S<1, 32, 1, 4>,        8>,
        DeviceConv2dBwdWeightXdl_C_Shuffle_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<    F16,     F16,     F16,     F32,  PassThrough, PassThrough, PassThrough,    256,   128,   128,    4,  8,   32,   32,    2,    2,  S<1, 4, 16, 4>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              2,    true,     S<1, 4, 16, 4>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              2,      true,        1,            1,     S<1, 32, 1, 4>,        8>,
        DeviceConv2dBwdWeightXdl_C_Shuffle_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<    F16,     F16,     F16,     F32,  PassThrough, PassThrough, PassThrough,    128,   128,    64,    4,  8,   32,   32,    2,    2,  S<1, 4, 16, 2>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              4,    true,     S<1, 4, 8,  4>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              2,      true,        1,            1,     S<1, 32, 1, 4>,        8>,
        DeviceConv2dBwdWeightXdl_C_Shuffle_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<    F16,     F16,     F16,     F32,  PassThrough, PassThrough, PassThrough,    128,    64,   128,    4,  8,   32,   32,    2,    2,  S<1, 4, 8,  4>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              2,    true,     S<1, 4, 16, 2>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              4,      true,        1,            1,     S<1, 32, 1, 4>,        8>,
        DeviceConv2dBwdWeightXdl_C_Shuffle_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<    F16,     F16,     F16,     F32,  PassThrough, PassThrough, PassThrough,     64,    64,    64,    4,  8,   32,   32,    2,    2,  S<1, 4, 8,  2>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              4,    true,     S<1, 4, 8,  2>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              4,      true,        1,            1,     S<1, 16, 1, 4>,        8>,
        DeviceConv2dBwdWeightXdl_C_Shuffle_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<    F16,     F16,     F16,     F32,  PassThrough, PassThrough, PassThrough,    256,   128,    64,    4,  8,   32,   32,    2,    1,  S<1, 4, 16, 4>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              2,    true,     S<1, 4, 8,  8>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              1,      true,        1,            1,     S<1, 32, 1, 4>,        8>,
        DeviceConv2dBwdWeightXdl_C_Shuffle_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<    F16,     F16,     F16,     F32,  PassThrough, PassThrough, PassThrough,    256,    64,   128,    4,  8,   32,   32,    1,    2,  S<1, 4, 8,  8>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              1,    true,     S<1, 4, 16, 4>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              2,      true,        1,            1,     S<1, 32, 1, 4>,        8>,
        DeviceConv2dBwdWeightXdl_C_Shuffle_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<    F16,     F16,     F16,     F32,  PassThrough, PassThrough, PassThrough,    128,   128,    32,    4,  8,   32,   32,    2,    1,  S<1, 4, 16, 2>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              4,    true,     S<1, 4, 4,  8>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              1,      true,        1,            1,     S<1, 32, 1, 4>,        8>,
        DeviceConv2dBwdWeightXdl_C_Shuffle_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<    F16,     F16,     F16,     F32,  PassThrough, PassThrough, PassThrough,    128,    32,   128,    4,  8,   32,   32,    1,    2,  S<1, 4, 4,  8>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              1,    true,     S<1, 4, 16, 2>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              4,      true,        1,            1,     S<1, 32, 1, 4>,        8>,
        DeviceConv2dBwdWeightXdl_C_Shuffle_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<    F16,     F16,     F16,     F32,  PassThrough, PassThrough, PassThrough,     64,    64,    32,    4,  8,   32,   32,    2,    1,  S<1, 4, 8,  2>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              4,    true,     S<1, 4, 4,  4>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              2,      true,        1,            1,     S<1, 16, 1, 4>,        8>,
        DeviceConv2dBwdWeightXdl_C_Shuffle_Input_N_Hi_Wi_C_Weight_K_Y_X_C_Output_N_Ho_Wo_K<    F16,     F16,     F16,     F32,  PassThrough, PassThrough, PassThrough,     64,    32,    64,    4,  8,   32,   32,    1,    2,  S<1, 4, 4,  4>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              2,    true,     S<1, 4, 8,  2>,   S<0, 3, 1, 2>,   S<0, 2, 1, 3>,             2,              8,              4,      true,        1,            1,     S<1, 16, 1, 4>,        8>
    // clang-format on
    >;

void add_device_conv2d_bwd_weight_xdl_nhwc_kyxc_nhwk_f16_instances(
    std::vector<DeviceConvBwdWeightPtr<PassThrough, PassThrough, PassThrough>>& instances)
{
    add_device_operation_instances(instances,
                                   device_conv2d_bwd_weight_xdl_nhwc_kyxc_nhwk_f16_instances{});
}

} // namespace device_conv2d_bwd_weight_instance
} // namespace device
} // namespace tensor_operation
} // namespace ck
