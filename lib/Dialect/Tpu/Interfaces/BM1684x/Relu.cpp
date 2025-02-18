//===----------------------------------------------------------------------===//
//
// Copyright (C) 2022 Sophgo Technologies Inc.  All rights reserved.
//
// TPU-MLIR is licensed under the 2-Clause BSD License except for the
// third-party components.
//
//===----------------------------------------------------------------------===//

#include "tpu_mlir/Dialect/Tpu/IR/TpuOps.h"
#include "tpu_mlir/Backend/BM168x/BM1684x.h"
#include "tpu_mlir/Support/Helper/Quant.h"
#include "tpu_mlir/Support/Helper/Module.h"

using namespace mlir;
using namespace tpu_mlir;
using namespace tpu_mlir::helper;
using namespace tpu_mlir::backend;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
// =========================================
// GlobalGenInterface
// =========================================

void tpu::ReluOp::codegen_global_bm1684x() {
  int64_t n, c, h, w;
  Module::getNCHW(output(), n, c, h, w);
  prelu_param_t p = {0};
  p.input_addr = Module::getAddress(input());
  p.slope_addr = 0;
  p.output_addr = Module::getAddress(output());
  p.input_n = n;
  p.input_c = c;
  p.input_h = h;
  p.input_w = w;
  p.channel_shared = true;
  p.slope_val = 0.f;
  p.rshift_bit = 0;
  p.relu_limit = relu_limit().convertToDouble();
  p.dtype = BM168x::getDataType(output());
  BM1684x::instance().call_global_func("backend_api_prelu_global", &p,
                                       sizeof(prelu_param_t));
}

// =========================================
// LocalGenInterface
// =========================================

int64_t tpu::ReluOp::getBufferSize_bm1684x(int64_t in_lmem_bytes,
                                           int64_t out_lmem_bytes,
                                           int64_t in_nslice, int64_t in_hslice,
                                           int64_t out_nslice,
                                           int64_t out_hslice) {
  return 0;
}

void tpu::ReluOp::codegen_local_bm1684x(int64_t n_step, int64_t h_step) {
  auto in_gi = LocalGenInterface::getGroupInfo(input(), n_step, h_step);
  auto out_gi = LocalGenInterface::getGroupInfo(output(), n_step, h_step);
  auto gi = getGroupInfo(n_step, h_step);
  int64_t n, c, h, w;
  Module::getNCHW(output(), n, c, h, w);

  prelu_param_t p = {0};
  p.input_addr = in_gi.out_addr;
  p.slope_addr = 0;
  p.output_addr = out_gi.out_addr;
  p.input_n = gi.n_slice;
  p.input_c = c;
  p.input_h = gi.h_slice;
  p.input_w = w;
  p.channel_shared = true;
  p.slope_val = 0.f;
  p.rshift_bit = 0;
  p.relu_limit = relu_limit().convertToDouble();
  p.dtype = BM168x::getDataType(output());
  BM1684x::instance().call_local_func("backend_api_prelu_local", &p,
                                      sizeof(prelu_param_t));
}
