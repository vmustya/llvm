//===--------- device.hpp - CUDA Adapter -----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===-----------------------------------------------------------------===//
#pragma once

#include <ur/ur.hpp>

struct ur_device_handle_t_ {
private:
  using native_type = CUdevice;

  native_type CuDevice;
  CUcontext CuContext;
  CUevent EvBase; // CUDA event used as base counter
  std::atomic_uint32_t RefCount;
  ur_platform_handle_t Platform;

  static constexpr uint32_t MaxWorkItemDimensions = 3u;
  size_t MaxWorkItemSizes[MaxWorkItemDimensions];
  size_t MaxWorkGroupSize{0};
  int MaxBlockDimY{0};
  int MaxBlockDimZ{0};
  int MaxRegsPerBlock{0};

public:
  ur_device_handle_t_(native_type cuDevice, CUcontext cuContext, CUevent evBase,
                      ur_platform_handle_t platform)
      : CuDevice(cuDevice), CuContext(cuContext), EvBase(evBase), RefCount{1},
        Platform(platform) {

    UR_CHECK_ERROR(cuDeviceGetAttribute(
        &MaxBlockDimY, CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y, cuDevice));
    UR_CHECK_ERROR(cuDeviceGetAttribute(
        &MaxBlockDimZ, CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z, cuDevice));
    UR_CHECK_ERROR(cuDeviceGetAttribute(
        &MaxRegsPerBlock, CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK,
        cuDevice));
  }

  ~ur_device_handle_t_() { cuDevicePrimaryCtxRelease(CuDevice); }

  native_type get() const noexcept { return CuDevice; };

  CUcontext getContext() const noexcept { return CuContext; };

  uint32_t getReferenceCount() const noexcept { return RefCount; }

  ur_platform_handle_t getPlatform() const noexcept { return Platform; };

  uint64_t getElapsedTime(CUevent) const;

  void saveMaxWorkItemSizes(size_t Size,
                            size_t *SaveMaxWorkItemSizes) noexcept {
    memcpy(MaxWorkItemSizes, SaveMaxWorkItemSizes, Size);
  };

  void saveMaxWorkGroupSize(int Value) noexcept { MaxWorkGroupSize = Value; };

  void getMaxWorkItemSizes(size_t RetSize,
                           size_t *RetMaxWorkItemSizes) const noexcept {
    memcpy(RetMaxWorkItemSizes, MaxWorkItemSizes, RetSize);
  };

  size_t getMaxWorkGroupSize() const noexcept { return MaxWorkGroupSize; };

  size_t getMaxBlockDimY() const noexcept { return MaxBlockDimY; };

  size_t getMaxBlockDimZ() const noexcept { return MaxBlockDimZ; };

  size_t getMaxRegsPerBlock() const noexcept { return MaxRegsPerBlock; };
};

int getAttribute(ur_device_handle_t Device, CUdevice_attribute Attribute);
