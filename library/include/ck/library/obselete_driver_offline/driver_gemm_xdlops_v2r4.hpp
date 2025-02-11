#ifndef DRIVER_GEMM_XDLOPS_V2R4
#define DRIVER_GEMM_XDLOPS_V2R4

#include "common_header.hpp"
#include "tensor_descriptor.hpp"
#include "tensor_descriptor_helper.hpp"
#include "gridwise_gemm_xdlops_v2r4.hpp"

template <ck::index_t BlockSize,
          typename FloatAB,
          typename FloatAcc,
          typename FloatC,
          ck::InMemoryDataOperationEnum CGlobalMemoryDataOperation,
          typename ABK0MK1GridDesc,
          typename BBK0NK1GridDesc,
          typename CMNGridDesc,
          ck::index_t MPerBlock,
          ck::index_t NPerBlock,
          ck::index_t KPerBlock,
          ck::index_t MPerXDL,
          ck::index_t NPerXDL,
          ck::index_t K1,
          ck::index_t MRepeat,
          ck::index_t NRepeat,
          typename ABlockTransferThreadSliceLengths_K0_M_K1,
          typename ABlockTransferThreadClusterLengths_K0_M_K1,
          typename ABlockTransferThreadClusterArrangeOrder,
          typename ABlockTransferSrcAccessOrder,
          ck::index_t ABlockTransferSrcVectorDim,
          ck::index_t ABlockTransferSrcScalarPerVector,
          ck::index_t ABlockTransferDstScalarPerVector_K1,
          bool AThreadTransferSrcResetCoordinateAfterRun,
          typename BBlockTransferThreadSliceLengths_K0_N_K1,
          typename BBlockTransferThreadClusterLengths_K0_N_K1,
          typename BBlockTransferThreadClusterArrangeOrder,
          typename BBlockTransferSrcAccessOrder,
          ck::index_t BBlockTransferSrcVectorDim,
          ck::index_t BBlockTransferSrcScalarPerVector,
          ck::index_t BBlockTransferDstScalarPerVector_K1,
          bool BThreadTransferSrcResetCoordinateAfterRun,
          typename CThreadTransferSrcDstAccessOrder,
          ck::index_t CThreadTransferSrcDstVectorDim,
          ck::index_t CThreadTransferDstScalarPerVector,
          typename AGridStepHacks,
          typename BGridStepHacks,
          typename CGridStepHacks,
          typename AGridMoveSliceWindowStepHacks,
          typename BGridMoveSliceWindowStepHacks,
          bool CAccessOrderMRepeatNRepeat,
          bool ABlockLdsAddExtraM,
          bool BBlockLdsAddExtraN>
__host__ float driver_gemm_xdlops_v2r4(const FloatAB* p_a_grid,
                                       const FloatAB* p_b_grid,
                                       FloatC* p_c_grid,
                                       const ABK0MK1GridDesc& a_b_k0_m_k1_grid_desc,
                                       const BBK0NK1GridDesc& b_b_k0_n_k1_grid_desc,
                                       const CMNGridDesc& c_m_n_grid_desc,
                                       ck::index_t M01,
                                       ck::index_t N01,
                                       AGridStepHacks,
                                       BGridStepHacks,
                                       CGridStepHacks,
                                       AGridMoveSliceWindowStepHacks,
                                       BGridMoveSliceWindowStepHacks,
                                       ck::index_t nrepeat)

{
    using namespace ck;

    constexpr auto I0 = Number<0>{};
    constexpr auto I1 = Number<1>{};
    constexpr auto I2 = Number<2>{};
    constexpr auto I3 = Number<3>{};

    using GridwiseGemm =
        GridwiseGemm_bk0mk1_bk0nk1_mn_xdlops_v2r4<BlockSize,
                                                  FloatAB,
                                                  FloatAcc,
                                                  FloatC,
                                                  CGlobalMemoryDataOperation,
                                                  ABK0MK1GridDesc,
                                                  BBK0NK1GridDesc,
                                                  CMNGridDesc,
                                                  MPerBlock,
                                                  NPerBlock,
                                                  KPerBlock,
                                                  MPerXDL,
                                                  NPerXDL,
                                                  K1,
                                                  MRepeat,
                                                  NRepeat,
                                                  ABlockTransferThreadSliceLengths_K0_M_K1,
                                                  ABlockTransferThreadClusterLengths_K0_M_K1,
                                                  ABlockTransferThreadClusterArrangeOrder,
                                                  ABlockTransferSrcAccessOrder,
                                                  ABlockTransferSrcVectorDim,
                                                  ABlockTransferSrcScalarPerVector,
                                                  ABlockTransferDstScalarPerVector_K1,
                                                  AThreadTransferSrcResetCoordinateAfterRun,
                                                  BBlockTransferThreadSliceLengths_K0_N_K1,
                                                  BBlockTransferThreadClusterLengths_K0_N_K1,
                                                  BBlockTransferThreadClusterArrangeOrder,
                                                  BBlockTransferSrcAccessOrder,
                                                  BBlockTransferSrcVectorDim,
                                                  BBlockTransferSrcScalarPerVector,
                                                  BBlockTransferDstScalarPerVector_K1,
                                                  BThreadTransferSrcResetCoordinateAfterRun,
                                                  CThreadTransferSrcDstAccessOrder,
                                                  CThreadTransferSrcDstVectorDim,
                                                  CThreadTransferDstScalarPerVector,
                                                  AGridStepHacks,
                                                  BGridStepHacks,
                                                  CGridStepHacks,
                                                  AGridMoveSliceWindowStepHacks,
                                                  BGridMoveSliceWindowStepHacks,
                                                  CAccessOrderMRepeatNRepeat,
                                                  ABlockLdsAddExtraM,
                                                  BBlockLdsAddExtraN>;

    {
        std::cout << "a_b_k0_m_k1_grid_desc{" << a_b_k0_m_k1_grid_desc.GetLength(I0) << ", "
                  << a_b_k0_m_k1_grid_desc.GetLength(I1) << ", "
                  << a_b_k0_m_k1_grid_desc.GetLength(I2) << ", "
                  << a_b_k0_m_k1_grid_desc.GetLength(I3) << "}" << std::endl;

        std::cout << "b_b_k0_n_k1_grid_desc{" << b_b_k0_n_k1_grid_desc.GetLength(I0) << ", "
                  << b_b_k0_n_k1_grid_desc.GetLength(I1) << ", "
                  << b_b_k0_n_k1_grid_desc.GetLength(I2) << ", "
                  << b_b_k0_n_k1_grid_desc.GetLength(I3) << "}" << std::endl;

        std::cout << "c_m_n_grid_desc{ " << c_m_n_grid_desc.GetLength(I0) << ", "
                  << c_m_n_grid_desc.GetLength(I1) << "}" << std::endl;
    }

    if(!GridwiseGemm::CheckValidity(
           a_b_k0_m_k1_grid_desc, b_b_k0_n_k1_grid_desc, c_m_n_grid_desc, M01, N01))
    {
        throw std::runtime_error(
            "wrong! GridwiseGemm_km_kn_m0m1n0n1_xdlops_v2r4 has invalid setting");
    }

    const auto c_m0_n0_m1_n1_m2_m3_m4_n2_grid_desc =
        GridwiseGemm::MakeCM0N0M1N1M2M3M4N2GridDescriptor(c_m_n_grid_desc);

    using CM0N0M1N1M2M3M4N2GridDesc = decltype(c_m0_n0_m1_n1_m2_m3_m4_n2_grid_desc);

    const auto KBatch = a_b_k0_m_k1_grid_desc.GetLength(I0);

    const auto c_block_cluster_adaptor =
        GridwiseGemm::MakeCBlockClusterAdaptor(c_m_n_grid_desc, M01, N01, KBatch);

    using CBlockClusterAdaptor = decltype(c_block_cluster_adaptor);

    const index_t grid_size = GridwiseGemm::CalculateGridSize(c_m_n_grid_desc, KBatch);
    {
        std::cout << "gridSize : " << grid_size << std::endl;
    }

    const auto K0 = a_b_k0_m_k1_grid_desc.GetLength(I1);

    const bool has_main_k0_block_loop = GridwiseGemm::CalculateHasMainK0BlockLoop(K0);

    float ave_time = 0;
    if(has_main_k0_block_loop)
    {
        const auto kernel = kernel_gemm_xdlops_v2r4<GridwiseGemm,
                                                    FloatAB,
                                                    FloatC,
                                                    remove_reference_t<ABK0MK1GridDesc>,
                                                    remove_reference_t<BBK0NK1GridDesc>,
                                                    remove_reference_t<CM0N0M1N1M2M3M4N2GridDesc>,
                                                    remove_reference_t<CBlockClusterAdaptor>,
                                                    true>;
        ave_time          = launch_and_time_kernel(kernel,
                                          nrepeat,
                                          dim3(grid_size),
                                          dim3(BlockSize),
                                          0,
                                          p_a_grid,
                                          p_b_grid,
                                          p_c_grid,
                                          a_b_k0_m_k1_grid_desc,
                                          b_b_k0_n_k1_grid_desc,
                                          c_m0_n0_m1_n1_m2_m3_m4_n2_grid_desc,
                                          c_block_cluster_adaptor);
    }
    else
    {
        const auto kernel = kernel_gemm_xdlops_v2r4<GridwiseGemm,
                                                    FloatAB,
                                                    FloatC,
                                                    remove_reference_t<ABK0MK1GridDesc>,
                                                    remove_reference_t<BBK0NK1GridDesc>,
                                                    remove_reference_t<CM0N0M1N1M2M3M4N2GridDesc>,
                                                    remove_reference_t<CBlockClusterAdaptor>,
                                                    false>;
        ave_time          = launch_and_time_kernel(kernel,
                                          nrepeat,
                                          dim3(grid_size),
                                          dim3(BlockSize),
                                          0,
                                          p_a_grid,
                                          p_b_grid,
                                          p_c_grid,
                                          a_b_k0_m_k1_grid_desc,
                                          b_b_k0_n_k1_grid_desc,
                                          c_m0_n0_m1_n1_m2_m3_m4_n2_grid_desc,
                                          c_block_cluster_adaptor);
    }

    return ave_time;
}
#endif
