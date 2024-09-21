// File: tint_wasm.cpp
// Author: Kyle Lukaszek
// Email: kylelukaszek [at] gmail [dot] com
// Date: 2024-09-21
// -------------------------------------------------------------
// Special thanks to the Tint contributors for their work on the Tint compiler.
// -------------------------------------------------------------
//
// License: Apache 2.0 (Follows the same license as Tint)
//
// Description: Module for interacting with common operations
//              performed using Tint & SPIRV-Tools. This module
//              is ideal for anyone looking to convert SPIRV to
//              WGSL or SPIRV ASM. This module is written in C++
//              and is meant to be compiled to WebAssembly using
//              Emscripten.
//
//              To compile this module to WebAssembly, use
//              the following command:
//
//              -------------------------------------------------
//
//        ->    make
//
//              -------------------------------------------------
//
//              This will generate a directory titled "build"
//              which will contain the WebAssembly binary, along
//              with the JavaScript wrapper, and a shell file to
//              test the WebAssembly binary.
//
//              This module is a work in progress and is not yet
//              complete. The goal is to provide a simple interface
//              for converting SPIRV to WGSL and SPIRV ASM using
//              Tint and SPIRV-Tools.
//
//              If you're not interested in the WASM module, you can
//              just grab the static library and plug it into your own
//              project. I've included the necessary headers within the
//              repo so you easily include them in your own projects.
//
// ---------------------------------------------------------------

#define TINT_BUILD_WGSL_WRITER 1
#define TINT_BUILD_WGSL_READER 1
#define TINT_BUILD_SPV_READER 1
#define TINT_BUILD_SPV_WRITER 1

#include "spirv-tools/libspirv.hpp"
#include "tint.h"
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <optional>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>

enum class Format : uint8_t {
  kUnknown,
  kNone,
  kSpirv,
  kSpvAsm,
  kWgsl,
  kIr,
  kIrBin
};

// Using static global variables to store the generated shader code
// resolves an issue with the WebAssembly module not returning the
// wgsl code specifically.
// I don't see an issue with this as this is a simple demo module.
static std::string wgsl_gen;
static std::string spv_asm_gen;
static std::vector<uint32_t> spv_bin_gen;

// SPIRV-Tools
static spvtools::SpirvTools spirv_tools(SPV_ENV_UNIVERSAL_1_6);

// Tint
static auto tint_spv_reader_options = tint::spirv::reader::Options{
    .allowed_features = tint::wgsl::AllowedFeatures::Everything(),
};
static tint::wgsl::writer::Options tint_wgsl_writer_options;

extern "C" {

// Takes a SPIRV binary file and converts it to SPIRV ASM
// using SPIRV-Tools
// Returns: C String
const char *SPV_TO_SPVASM(const uint32_t *spirv, size_t size) {
  // This automatically disassembles the SPIRV binary file
  // and stores the generated SPIRV ASM in our static global variable
  if (!spirv_tools.Disassemble(spirv, size, &spv_asm_gen,
                               SPV_BINARY_TO_TEXT_OPTION_NONE)) {
    throw std::runtime_error("Failed to disassemble SPIRV");
  }

  // Convert SPIRV to SPIRV ASM
  return spv_asm_gen.c_str();
}

// When called by JS, we will need to prompt the user to download the
// generated SPIRV binary file. This is because the printed SPIRV binary
// would just be garbage text when copied and pasted.
// Returns: C String
const void *SPVASM_TO_SPV(const char *spv_asm, size_t size) {
  // This automatically assembles the SPIRV ASM file
  // and stores the generated SPIRV binary in our static global variable
  if (!spirv_tools.Assemble(spv_asm, size, &spv_bin_gen,
                            SPV_TEXT_TO_BINARY_OPTION_NONE)) {
    throw std::runtime_error("Failed to assemble SPIRV ASM");
  }

  // Convert SPIRV ASM to SPIRV
  return spv_bin_gen.data();
}

// Takes a SPIRV binary file and converts it to WGSL
// using Tint
// Returns: String
const char *SPV_TO_WGSL(uint32_t *spirv, size_t size) {

  // Parse the SPIRV binary file into an AST
  std::vector<uint32_t> spirv_binary(spirv, spirv + size);
  auto program =
      tint::spirv::reader::Read(spirv_binary, tint_spv_reader_options);
  if (!program.IsValid()) {
    std::cerr << "Failed to parse SPIRV file" << std::endl;
    return nullptr;
  }

  // Generate WGSL from the AST
  auto result = tint::wgsl::writer::Generate(program, tint_wgsl_writer_options);
  if (result != tint::Success) {
    std::cerr << "Failed to generate WGSL" << std::endl;
    return nullptr;
  }

  // Store the generated WGSL code in our static global variable
  wgsl_gen = result->wgsl;

  return wgsl_gen.c_str();
}

} // extern "C"

/*// Attempt to re-parse the output program with Tint's WGSL reader.*/
/*tint::wgsl::reader::Options parser_options;*/
/*parser_options.allowed_features = tint::wgsl::AllowedFeatures::Everything();*/
/*auto source =*/
/*    std::make_unique<tint::Source::File>("output.wgsl", result->wgsl);*/
/*auto reparsed_program =*/
/*    tint::wgsl::reader::Parse(source.get(), parser_options);*/
/*if (!reparsed_program.IsValid()) {*/
/*  std::cerr << "Failed to re-parse generated WGSL: " << std::endl;*/
/*} else {*/
/*  std::cout << "Successfully re-parsed generated WGSL" << std::endl;*/
/*}*/
/**/
/*// Print the binding information from the re-parsed program.*/
/*tint::inspector::Inspector inspector(reparsed_program);*/
/*/*auto bindings = inspector.GetEntryPoint*/
