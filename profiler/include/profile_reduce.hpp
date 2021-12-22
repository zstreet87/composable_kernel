#pragma once
#include "device_reduce_instance.hpp"
#include "reduction_enums.hpp"

namespace ck {
namespace tensor_operation {
namespace device {
namespace device_reduce_instance {

template <int rank, typename toReduceDims, int reduceOp, int nanOpt, int indicesOpt> 
struct ReduceDescription
{
    static constexpr int rank_ = rank; 
    static constexpr int reduceOp_ = reduceOp; 
    static constexpr int nanOpt_ = nanOpt; 
    static constexpr int indicesOpt_ = indicesOpt; 

    using toReduceDims_ = toReduceDims; 
}; 

using reduce_description_instances = std::tuple< 
    ReduceDescription<4, Sequence<0,1,2>, 0, 0, 0>,                        // for ADD
    ReduceDescription<4, Sequence<0>, 0, 0, 0>, 
    ReduceDescription<4, Sequence<0,1,2>, 1, 0, 0>,                        // for MUL
    ReduceDescription<4, Sequence<0>, 1, 0, 0>, 
    ReduceDescription<4, Sequence<0,1,2>, 5, 0, 0>,                        // for AVG
    ReduceDescription<4, Sequence<0>, 0, 5, 0>, 
    ReduceDescription<4, Sequence<0,1,2>, 6, 0, 0>,                        // for NORM1
    ReduceDescription<4, Sequence<0>, 0, 6, 0>, 
    ReduceDescription<4, Sequence<0,1,2>, 7, 0, 0>,                        // for NORM2
    ReduceDescription<4, Sequence<0>, 7, 0, 0>, 

    ReduceDescription<4, Sequence<0,1,2>, 2, 0, 0>,                        // for MIN
    ReduceDescription<4, Sequence<0>, 2, 0, 0>,
    ReduceDescription<4, Sequence<0,1,2>, 3, 0, 0>,                        // for MAX
    ReduceDescription<4, Sequence<0>, 3, 0, 0>,
    ReduceDescription<4, Sequence<0,1,2>, 4, 0, 0>,                        // for AMAX
    ReduceDescription<4, Sequence<0>, 4, 0, 0>,

    ReduceDescription<4, Sequence<0,1,2>, 2, 0, 1>,                        // for MIN
    ReduceDescription<4, Sequence<0>, 2, 0, 1>, 
    ReduceDescription<4, Sequence<0,1,2>, 3, 0, 1>,                        // for MAX
    ReduceDescription<4, Sequence<0>, 3, 0, 1>, 
    ReduceDescription<4, Sequence<0,1,2>, 4, 0, 1>,                        // for AMAX
    ReduceDescription<4, Sequence<0>, 4, 0, 1>    
    >;

template <typename DescriptionType>
bool description_match(const DescriptionType & description, int rank, const std::vector<int>& toReduceDims, ReduceTensorOp_t reduceOp, NanPropagation_t nanOpt, ReduceTensorIndices_t indicesOpt)
{
     if ( description.rank_ != rank || description.reduceOp_ != static_cast<int>(reduceOp) || description.nanOpt_ != static_cast<int>(nanOpt) || description.indicesOpt_ != static_cast<int>(indicesOpt) ) 
	 return(false); 

     if (DescriptionType::toReduceDims_::Size() != toReduceDims.size())
	 return(false); 

     bool result = true; 

     static_for<0, DescriptionType::toReduceDims_::Size(), 1>{}( [&](auto i) {
          if (DescriptionType::toReduceDims_::At(i) != toReduceDims[i])
	      result = false; 
     }); 

     return(false); 
}; 

} // namespace device_reduce_instance
} // namespace device
} // namespace tensor_operation
} // namespace ck

namespace ck {
namespace profiler {

template <int rank, typename toReduceDims>
static std::vector<int> get_toReduce_dims()
{
    std::vector<int> resDims;

    static_for<0, toReduceDims::Size(), 1>{}( [&](auto i) {
        resDims.push_back(toReduceDims::At(i));
    }); 

    return(resDims); 
};

template <int rank, typename toReduceDims>
static std::vector<int> get_invariant_dims()
{
    std::vector<int> resDims;
    unsigned int incFlag = 0;

    static_for<0, toReduceDims::Size(), 1>{}( [&](auto i) {
        incFlag = incFlag | (0x1 << toReduceDims::At(i));
    }); 

    for(int dim = 0; dim < rank; dim++) {
        if(incFlag & (0x1 << dim))
            continue;
        resDims.push_back(dim);
    };

    return(resDims);
};

static std::vector<int> to_int_vector(const std::vector<size_t> & inData)
{
    std::vector<int> outData; 

    for (auto elem : inData) 
	 outData.push_back(static_cast<int>(elem)); 

    return(outData); 
}; 

static void check_indices(const Tensor<int>& ref, const Tensor<int>& result)
{
    bool has_error  = false;
    int error_count = 0;

    for(int i = 0; i < ref.mData.size(); ++i) {
        if(ref.mData[i] != result.mData[i]) {
            std::cerr << std::endl << "Indices different at position " << i << " (ref: " << ref.mData[i]
                      << ", result: " << result.mData[i] << ")" << std::endl;
            has_error = true;
            error_count++;
            if(error_count == 20)
                break;
        };
    }

    if(!has_error)
        std::cout << std::endl << "Indices result is completely acccurate!" << std::endl;
}

template <typename inType, typename compType, typename outType, 
         int rank_, typename toReduceDims_, ReduceTensorOp_t reduceOp, NanPropagation_t nanOpt, ReduceTensorIndices_t indicesOpt>
void profile_reduce_impl(bool do_verification, int init_method, bool do_log, int nrepeat, const std::vector<size_t> & inLengths, float alpha, float beta)
{
    Tensor<inType> in(inLengths);

    std::vector<int> invariantDims;
    std::vector<int> toReduceDims; 

    std::vector<size_t> outLengths;

    toReduceDims = get_toReduce_dims<rank_, toReduceDims_>(); 
    invariantDims = get_invariant_dims<rank_, toReduceDims_>();

    if (invariantDims.empty())
        outLengths.push_back(1);
    else
        for (auto dim : invariantDims)
             outLengths.push_back(inLengths[dim]);

    auto inStrides  = in.mDesc.GetStrides();

    Tensor<outType> out_host(outLengths);
    Tensor<outType> out_dev(outLengths);
    Tensor<int> out_indices_host(outLengths);
    Tensor<int> out_indices_dev(outLengths);

    auto outStrides = out_dev.mDesc.GetStrides();

    // used for mapping to a configuraton instance
    int dim0_length = invariantDims.empty()? 1 : inLengths[invariantDims.back()]; 
    int dim0_stride = invariantDims.empty()? 1 : inStrides[invariantDims.back()]; 
    int dim1_length = inLengths[toReduceDims.back()]; 
    int dim1_stride = inStrides[toReduceDims.back()]; 
    size_t dim0_total_length = out_dev.mDesc.GetElementSize();
    size_t dim1_total_length = in.mDesc.GetElementSize() / dim0_total_length;

    std::size_t num_thread = std::thread::hardware_concurrency();

    bool need_indices = ((indicesOpt != ReduceTensorIndices_t::NO_INDICES) &&
		         (reduceOp == ReduceTensorOp_t::MIN || reduceOp == ReduceTensorOp_t::MAX || reduceOp == ReduceTensorOp_t::AMAX));

    if(do_verification)
    {
        switch(init_method)
        {
        case 0:
            in.GenerateTensorValue(GeneratorTensor_1<inType>{}, num_thread);
            if(beta != 0.0f)
                out_host.GenerateTensorValue(GeneratorTensor_1<inType>{}, num_thread);
            break;
        case 1:
            in.GenerateTensorValue(GeneratorTensor_2<inType>{-99, 99}, num_thread);
            if(beta != 0.0f)
                out_host.GenerateTensorValue(GeneratorTensor_2<inType>{-5, 5}, num_thread);
            break;
        default:
            in.GenerateTensorValue(GeneratorTensor_2<inType>{1, 5}, num_thread);
            if(beta != 0.0f)
                out_host.GenerateTensorValue(GeneratorTensor_2<inType>{1, 5}, num_thread);
        }

        if(beta != 0.0f)
            for(size_t i = 0; i < out_host.mDesc.GetElementSpace(); i++)
                out_dev.mData[i] = out_host.mData[i];
    };

    if(do_verification)
    {
         ReductionHost<inType, compType, outType> hostReduce(reduceOp, nanOpt, indicesOpt, in.mDesc, out_host.mDesc, invariantDims, toReduceDims);

         hostReduce.Run(alpha, in.mData.data(), beta, out_host.mData.data(), out_indices_host.mData.data());
    }; 

    // these buffers are usually provided by the user application
    DeviceMem in_dev_buf(sizeof(inType) * in.mDesc.GetElementSpace());
    DeviceMem out_dev_buf(sizeof(outType) * out_host.mDesc.GetElementSpace());

    in_dev_buf.ToDevice(in.mData.data());

    if(beta != 0.0f)
        out_dev_buf.ToDevice(out_host.mData.data());

    size_t indicesSizeInBytes = need_indices ? out_host.mDesc.GetElementSize() * sizeof(int) : 0;

    DeviceMem out_indices_dev_buf(indicesSizeInBytes);

    float best_avg_time   = 0;
    float best_gb_per_sec = 0;

    using DeviceReduceInstPtr = ck::tensor_operation::device::DeviceReducePtr<inType, compType, outType, rank_, toReduceDims_, reduceOp, nanOpt, indicesOpt>; 
    using DeviceReduceInstPtr2 = ck::tensor_operation::device::DeviceReducePtr<compType, compType, outType, rank_, toReduceDims_, reduceOp, nanOpt, indicesOpt>; 

    std::vector<DeviceReduceInstPtr> reduce_ptrs;
    std::vector<DeviceReduceInstPtr2> reduce2_ptrs;

    ck::tensor_operation::device::device_reduce_instance::add_device_reduce_instance_threadwise<inType, compType, outType, rank_, toReduceDims_, reduceOp, nanOpt, indicesOpt>(reduce_ptrs); 
    ck::tensor_operation::device::device_reduce_instance::add_device_reduce_instance_blockwise<inType, compType, outType, rank_, toReduceDims_, reduceOp, nanOpt, indicesOpt>(reduce_ptrs); 
    ck::tensor_operation::device::device_reduce_instance::add_device_reduce_instance_multiblock_atomic_add<inType, compType, outType, rank_, toReduceDims_, reduceOp, nanOpt, indicesOpt>(reduce_ptrs); 
    ck::tensor_operation::device::device_reduce_instance::add_device_reduce_instance_multiblock_two_call<inType, compType, outType, rank_, toReduceDims_, reduceOp, nanOpt, indicesOpt>(reduce_ptrs); 

    // used for secondary reduction
    ck::tensor_operation::device::device_reduce_instance::add_device_reduce_instance_blockwise_second_call<compType, compType, outType, rank_, toReduceDims_, reduceOp, nanOpt, indicesOpt>(reduce2_ptrs); 

    if(reduce_ptrs.empty())
    {
        throw std::runtime_error("Wrong! No device REDUCE instance found");
    };

    for(auto& reduce_ptr : reduce_ptrs) 
    {
         const auto i_inLengths = to_int_vector(inLengths); 
	 const auto i_inStrides = to_int_vector(inStrides); 
	 const auto i_outLengths = to_int_vector(outLengths); 
	 const auto i_outStrides = to_int_vector(outStrides); 

         auto wsSizeInBytes = reduce_ptr->getWorkspaceSize(i_inLengths); 

         DeviceMem ws_dev_buf(wsSizeInBytes); 

         auto argument_ptr = reduce_ptr->MakeArgumentPointer(i_inLengths, i_inStrides, i_outLengths, i_outStrides, alpha, beta,  
			                          in_dev_buf.GetDeviceBuffer(), out_dev_buf.GetDeviceBuffer(), out_indices_dev_buf.GetDeviceBuffer(), ws_dev_buf.GetDeviceBuffer()); 

         if( !reduce_ptr->IsSupportedArgument(argument_ptr.get()) )
	      continue; 

         auto invoker_ptr = reduce_ptr->MakeInvokerPointer();

         float avg_time = invoker_ptr->Run(argument_ptr.get(), nrepeat);

         std::size_t num_bytes = dim0_total_length * dim1_total_length * sizeof(inType) + dim0_total_length * sizeof(outType); 

         if(reduce_ptr->hasFurtherCall()) {
             std::vector<int> inLengths2 = reduce_ptr->getWorkspace2dLengths(argument_ptr.get());    
	     std::vector<int> inStrides2{inLengths2[1], 1}; 

	     for(auto& reduce2_ptr : reduce2_ptrs) {
                 auto argument2_ptr = reduce2_ptr->MakeArgumentPointer(inLengths2, inStrides2, i_outLengths, i_outStrides, alpha, beta,  
			                          ws_dev_buf.GetDeviceBuffer(), out_dev_buf.GetDeviceBuffer(), out_indices_dev_buf.GetDeviceBuffer(), nullptr); 

                 auto invoker2_ptr = reduce2_ptr->MakeInvokerPointer();

                 if ( !reduce2_ptr->IsSupportedArgument(argument2_ptr.get()) )
	              continue; 
		      
                 float avg_time_2 = invoker2_ptr->Run(argument2_ptr.get(), nrepeat); 

                 std::size_t num_bytes_2 = static_cast<size_t>(inLengths2[0]) * inLengths2[1] * sizeof(compType); 

		 float gb_per_sec = (num_bytes + num_bytes_2) / 1.E6 / (avg_time + avg_time_2); 

                 std::cout << "Perf: " << (avg_time+avg_time_2) << " ms, " << gb_per_sec << " GB/s" << std::endl;

                 if(gb_per_sec > best_gb_per_sec) {
                     best_avg_time   = avg_time + avg_time_2;
                     best_gb_per_sec = gb_per_sec;
                 }

                 if(do_verification) {
                     ReductionHost<inType, compType, outType> hostReduce(reduceOp, nanOpt, indicesOpt,
                                                 in.mDesc, out_host.mDesc, invariantDims, toReduceDims);

                     hostReduce.Run(alpha, in.mData.data(), beta, out_host.mData.data(), out_indices_host.mData.data());


                     out_dev_buf.FromDevice(out_dev.mData.data());
                     check_error(out_host, out_dev);
                      
                     if(need_indices) {
                         out_indices_dev_buf.FromDevice(out_indices_dev.mData.data());
                         check_indices(out_indices_host, out_indices_dev);
                     };
                      
                     if(do_log) {  
                         /*
                         LogRange(std::cout << "out_host  : ", out_host.mData, ",") << std::endl;
                         LogRange(std::cout << "out_device: ", out_dev.mData, ",") << std::endl;
                         */
                     }
		 } 
	     }; 
	 }  
         else {
             float gb_per_sec = num_bytes / 1.E6 / avg_time; 

             std::cout << "Perf: " << avg_time << " ms, " << gb_per_sec << " GB/s" << std::endl;

             if(gb_per_sec > best_gb_per_sec) {
                 best_avg_time   = avg_time;
                 best_gb_per_sec = gb_per_sec;
             }

             if(do_verification)
             {
                 ReductionHost<inType, compType, outType> hostReduce(reduceOp, nanOpt, indicesOpt,
                                                  in.mDesc, out_host.mDesc, invariantDims, toReduceDims);

                 hostReduce.Run(alpha, in.mData.data(), beta, out_host.mData.data(), out_indices_host.mData.data());

                 out_dev_buf.FromDevice(out_dev.mData.data());
                 check_error(out_host, out_dev);

                 if(need_indices) {
                     out_indices_dev_buf.FromDevice(out_indices_dev.mData.data());
                     check_indices(out_indices_host, out_indices_dev);
                 }; 

                 if(do_log) {
/*
                     LogRange(std::cout << "out_host  : ", out_host.mData, ",") << std::endl;
                     LogRange(std::cout << "out_device: ", out_dev.mData, ",") << std::endl;
*/		     
                 }; 
	     }; 
         } 
    };

    std::cout << "Best Perf: " << best_avg_time << " ms, " << best_gb_per_sec << " GB/s" << std::endl;       
}; 

template <typename inType, typename compType, typename outType>
void profile_reduce(bool do_verification, int init_method, bool do_log, int nrepeat,
		  const std::vector<size_t> & inLengths, const std::vector<int> & toReduceDims, 
		  ReduceTensorOp_t reduceOp, NanPropagation_t nanOpt, ReduceTensorIndices_t indicesOpt, 
		  float alpha, float beta)
{
    bool matched = false; 

    using tuple_of_description_instances = tensor_operation::device::device_reduce_instance::reduce_description_instances;

    const auto tuple_object = tuple_of_description_instances{}; 

    static_for<0, std::tuple_size<tuple_of_description_instances>::value, 1>{}( [&](auto i) {
          if (matched)
	      return; 

          using descType = remove_cvref_t<decltype(std::get<i>(tuple_object))>; 

          if ( !description_match(descType{}, inLengths.size(), toReduceDims, reduceOp, nanOpt, indicesOpt) )
		 return; 

          profile_reduce_impl<inType, compType, outType, descType::rank_, typename descType::toReduceDims_, static_cast<ReduceTensorOp_t>(descType::reduceOp_), 
	                             static_cast<NanPropagation_t>(descType::nanOpt_), static_cast<ReduceTensorIndices_t>(descType::indicesOpt_)>(
                 do_verification, init_method, do_log, nrepeat, inLengths, alpha, beta);

          matched = true; 
    }); 
};

} // namespace profiler
} // namespace ck
