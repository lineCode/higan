template<typename Target, typename Source>
auto TLCS900H::instructionAdd(Target target, Source source) -> void {
  store(target, algorithmAdd(load(target), load(source)));
}

template<typename Target, typename Source>
auto TLCS900H::instructionAddCarry(Target target, Source source) -> void {
  store(target, algorithmAdd(load(target), load(source), CF));
}

template<typename Target, typename Source>
auto TLCS900H::instructionAnd(Target target, Source source) -> void {
  store(target, algorithmAnd(load(target), load(source)));
}

template<typename Source, typename Offset>
auto TLCS900H::instructionAndCarry(Source source, Offset offset) -> void {
  if constexpr(Source::bits == 8 && is_same_v<Offset, Register<uint8>>) { if(load(offset).bit(3)) return (void)Undefined; }
  CF &= load(source).bit(load(offset) & Source::bits - 1);
}

template<typename Source, typename Offset>
auto TLCS900H::instructionBit(Source source, Offset offset) -> void {
  NF = 0;
  VF = Undefined;
  HF = 1;
  ZF = !load(source).bit(load(offset) & Source::bits - 1);
  SF = Undefined;
}

auto TLCS900H::instructionBitSearch1Backward(Register<uint16> register) -> void {
  auto value = load(register);
  for(uint index : reverse(range(16))) {
    if(value.bit(index)) return VF = 1, store(A, index);
  }
  VF = 0;
}

auto TLCS900H::instructionBitSearch1Forward(Register<uint16> register) -> void {
  auto value = load(register);
  for(uint index : range(16)) {
    if(value.bit(index)) return VF = 1, store(A, index);
  }
  VF = 0;
}

template<typename Source>
auto TLCS900H::instructionCall(uint4 code, Source source) -> void {
  auto address = load(source);
  if(condition(code)) push(PC), store(PC, address);
}

template<typename Source>
auto TLCS900H::instructionCallRelative(Source displacement) -> void {
  push(PC);
  store(PC, load(PC) + load(displacement));
}

template<typename Target, typename Offset>
auto TLCS900H::instructionChange(Target target, Offset offset) -> void {
  auto result = load(target);
  result.bit(load(offset) & Target::bits - 1) ^= 1;
  store(target, result);
}

template<typename Target, typename Source>
auto TLCS900H::instructionCompare(Target target, Source source) -> void {
  algorithmSubtract(load(target), load(source));
}

template<typename Target>
auto TLCS900H::instructionComplement(Target target) -> void {
  store(target, ~load(target));
  NF = 1;
  HF = 1;
}

auto TLCS900H::instructionDecimalAdjustAccumulator(Register<uint8> register) -> void {
  auto value = load(register);
  if(CF || (uint8)value > 0x99) value += NF ? -0x60 : 0x60, CF = 1;
  if(HF || (uint4)value > 0x09) value += NF ? -0x06 : 0x06;
  PF = parity(value);
  HF = uint8(value ^ load(register)).bit(4);
  ZF = value == 0;
  SF = value.bit(-1);
  store(register, value);
}

template<typename Target, typename Source>
auto TLCS900H::instructionDecrement(Target target, Source source) -> void {
  auto immediate = load(source);
  if(!immediate) immediate = 8;
  store(target, algorithmDecrement(load(target), immediate));
}

template<typename Target, typename Source>
auto TLCS900H::instructionDivide(Target target, Source source) -> void {
  using T = typename Target::type;
  using E = Natural<2 * T::bits()>;
  auto dividend  = load(expand(target));
  auto divisor   = load(source);
  auto quotient  = divisor ? E(dividend / divisor) : E(T(~(dividend >> T::bits())));
  auto remainder = divisor ? E(dividend % divisor) : E(T(dividend));
  store(expand(target), T(remainder) << T::bits() | T(quotient));
  VF = !divisor || remainder >> T::bits();
}

template<typename Target, typename Source>
auto TLCS900H::instructionDivideSigned(Target target, Source source) -> void {
  using T = typename Target::type;
  using E = Natural<2 * T::bits()>;
  auto dividend  = load(expand(target)).integer();
  auto divisor   = load(source).integer();
  auto quotient  = divisor ? E(dividend / divisor) : E(T(~(dividend >> T::bits())));
  auto remainder = divisor ? E(dividend % divisor) : E(T(dividend));
  store(expand(target), T(remainder) << T::bits() | T(quotient));
  VF = !divisor || remainder >> T::bits();
}

template<typename Target, typename Source>
auto TLCS900H::instructionExchange(Target target, Source source) -> void {
  auto data = load(target);
  store(target, load(source));
  store(source, data);
}

template<typename Target>
auto TLCS900H::instructionExtendSign(Target target) -> void {
  store(target, load(shrink(target)).integer());
}

template<typename Target>
auto TLCS900H::instructionExtendZero(Target target) -> void {
  store(target, load(shrink(target)));
}

auto TLCS900H::instructionHalt() -> void {
  setHalted(true);
}

template<typename Target, typename Source>
auto TLCS900H::instructionIncrement(Target target, Source source) -> void {
  auto immediate = load(source);
  if(!immediate) immediate = 8;
  store(target, algorithmIncrement(load(target), immediate));
}

template<typename Source>
auto TLCS900H::instructionJump(uint4 code, Source source) -> void {
  auto address = load(source);
  if(condition(code)) store(PC, address);
}

template<typename Source>
auto TLCS900H::instructionJumpRelative(uint4 code, Source displacement) -> void {
  if(condition(code)) store(PC, load(PC) + load(displacement));
}

template<typename Target, typename Source>
auto TLCS900H::instructionLoad(Target target, Source source) -> void {
  store(target, load(source));
}

template<typename Source, typename Offset>
auto TLCS900H::instructionLoadCarry(Source source, Offset offset) -> void {
  if constexpr(Source::bits == 8 && is_same_v<Offset, Register<uint8>>) { if(load(offset).bit(3)) return (void)Undefined; }
  CF = load(source).bit(load(offset) & Source::bits - 1);
}

//reverse all bits in a 16-bit register
//note: an 8-bit lookup table is faster (when in L1/L2 cache), but much more code
auto TLCS900H::instructionMirror(Register<uint16> register) -> void {
  auto data = load(register);
  uint8 lo = (data.byte(0) * 0x80200802ull & 0x884422110ull) * 0x101010101ull >> 32;
  uint8 hi = (data.byte(1) * 0x80200802ull & 0x884422110ull) * 0x101010101ull >> 32;
  store(register, lo << 8 | hi << 0);
}

template<typename Target, typename Source>
auto TLCS900H::instructionMultiply(Target target, Source source) -> void {
  store(expand(target), load(target) * load(source));
}

auto TLCS900H::instructionMultiplyAdd(Register<uint16> register) -> void {
  auto xde = toMemory<int16>(load(XDE));
  auto xhl = toMemory<int16>(load(XHL));

  auto source = load(expand(register));
  auto target = load(xde) * load(xhl);
  store(expand(register), source + target);
  store(XHL, load(XHL) - 2);

  auto result = load(expand(register));
  VF = uint32((target ^ result) & (source ^ result)).bit(-1);
  ZF = result == 0;
  SF = result.bit(-1);
}

template<typename Target, typename Source>
auto TLCS900H::instructionMultiplySigned(Target target, Source source) -> void {
  store(expand(target), load(target).integer() * load(source).integer());
}

template<typename Target>
auto TLCS900H::instructionNegate(Target target) -> void {
  store(target, algorithmSubtract(typename Target::type{0}, load(target)));
}

auto TLCS900H::instructionNoOperation() -> void {
}

template<typename Target, typename Source>
auto TLCS900H::instructionOr(Target target, Source source) -> void {
  store(target, algorithmOr(load(target), load(source)));
}

template<typename Source, typename Offset>
auto TLCS900H::instructionOrCarry(Source source, Offset offset) -> void {
  if constexpr(Source::bits == 8 && is_same_v<Offset, Register<uint8>>) { if(load(offset).bit(3)) return (void)Undefined; }
  CF |= load(source).bit(load(offset) & Source::bits - 1);
}

template<typename Target>
auto TLCS900H::instructionPop(Target target) -> void {
  pop(target);
}

template<typename Source>
auto TLCS900H::instructionPush(Source source) -> void {
  push(source);
}

template<typename Target, typename Offset>
auto TLCS900H::instructionReset(Target target, Offset offset) -> void {
  auto result = load(target);
  result.bit(load(offset) & Target::bits - 1) = 0;
  store(target, result);
}

auto TLCS900H::instructionReturn(uint4 code) -> void {
  if(condition(code)) pop(PC);
}

template<typename Source>
auto TLCS900H::instructionReturnDeallocate(Source displacement) -> void {
  pop(PC);
  store(XSP, load(XSP) + load(displacement));
}

auto TLCS900H::instructionReturnInterrupt() -> void {
  pop(SR);
  pop(PC);
  store(INTNEST, load(INTNEST) - 1);
}

template<typename Target, typename Amount>
auto TLCS900H::instructionRotateLeft(Target target, Amount amount) -> void {
  auto result = load(target);
  uint count = (uint4)load(amount);
  for(uint n : range(if(count, 16))) {
    uint cf = result.bit(-1);
    result = result << 1 | CF;
    CF = cf;
  }
  store(target, algorithmRotated(result));
}

template<typename Target, typename Amount>
auto TLCS900H::instructionRotateLeftWithoutCarry(Target target, Amount amount) -> void {
  auto result = load(target);
  uint count = (uint4)load(amount);
  for(uint n : range(if(count, 16))) {
    CF = result.bit(-1);
    result = result << 1 | CF;
  }
  store(target, algorithmRotated(result));
}

template<typename Target, typename Amount>
auto TLCS900H::instructionRotateRight(Target target, Amount amount) -> void {
  auto result = load(target);
  uint count = (uint4)load(amount);
  for(uint n : range(if(count, 16))) {
    uint cf = result.bit(0);
    result = CF << Target::bits - 1 | result >> 1;
    CF = cf;
  }
  store(target, algorithmRotated(result));
}

template<typename Target, typename Amount>
auto TLCS900H::instructionRotateRightWithoutCarry(Target target, Amount amount) -> void {
  auto result = load(target);
  uint count = (uint4)load(amount);
  for(uint n : range(if(count, 16))) {
    CF = result.bit(0);
    result = CF << Target::bits - 1 | result >> 1;
  }
  store(target, algorithmRotated(result));
}

template<typename Target, typename Offset>
auto TLCS900H::instructionSet(Target target, Offset offset) -> void {
  auto result = load(target);
  result.bit(load(offset) & Target::bits - 1) = 1;
  store(target, result);
}

template<typename Target>
auto TLCS900H::instructionSetConditionCode(uint4 code, Target target) -> void {
  store(target, condition(code));
}

auto TLCS900H::instructionSetFlag(uint1& flag, uint1 value) -> void {
  flag = value;
}

auto TLCS900H::instructionSetInterruptFlipFlop(uint3 value) -> void {
  IFF = value;
}

auto TLCS900H::instructionSetRegisterFilePointer(uint2 value) -> void {
  RFP = value;
}

template<typename Target, typename Amount>
auto TLCS900H::instructionShiftLeftArithmetic(Target target, Amount amount) -> void {
  auto result = load(target);
  uint count = (uint4)load(amount);
  for(uint n : range(if(count, 16))) {
    CF = result.bit(-1);
    result = result << 1;
  }
  store(target, algorithmShifted(result));
}

template<typename Target, typename Amount>
auto TLCS900H::instructionShiftLeftLogical(Target target, Amount amount) -> void {
  auto result = load(target);
  uint count = (uint4)load(amount);
  for(uint n : range(if(count, 16))) {
    CF = result.bit(-1);
    result = result << 1;
  }
  store(target, algorithmShifted(result));
}

template<typename Target, typename Amount>
auto TLCS900H::instructionShiftRightArithmetic(Target target, Amount amount) -> void {
  auto result = load(target);
  uint count = (uint4)load(amount);
  for(uint n : range(if(count, 16))) {
    CF = result.bit(0);
    result = result >> 1;
    result.bit(-1) = result.bit(-2);
  }
  store(target, algorithmShifted(result));
}

template<typename Target, typename Amount>
auto TLCS900H::instructionShiftRightLogical(Target target, Amount amount) -> void {
  auto result = load(target);
  uint count = (uint4)load(amount);
  for(uint n : range(if(count, 16))) {
    CF = result.bit(0);
    result = result >> 1;
  }
  store(target, algorithmShifted(result));
}

template<typename Target, typename Offset>
auto TLCS900H::instructionStoreCarry(Target target, Offset offset) -> void {
  if constexpr(Target::bits == 8) { if(load(offset).bit(3)) return; }  //unlike other *CF instructions, STCF behavior is defined
  auto result = load(target);
  result.bit(load(offset)) = CF;
  store(target, result);
}

auto TLCS900H::instructionSoftwareInterrupt(uint3 interrupt) -> void {
  //TODO
}

template<typename Target, typename Source>
auto TLCS900H::instructionSubtract(Target target, Source source) -> void {
  store(target, algorithmSubtract(load(target), load(source)));
}

//note: the TLCS900/H manual states this is subtract-with-carry, but it isn't
template<typename Target, typename Source>
auto TLCS900H::instructionSubtractBorrow(Target target, Source source) -> void {
  store(target, algorithmSubtract(load(target), load(source), CF));
}

template<typename Target, typename Offset>
auto TLCS900H::instructionTestSet(Target target, Offset offset) -> void {
  auto result = load(target);
  NF = 0;
  VF = Undefined;
  HF = 1;
  ZF = result.bit(load(offset) & Target::bits - 1);
  SF = Undefined;
  result.bit(load(offset) & Target::bits - 1) = 1;
  store(target, result);
}

template<typename Target, typename Source>
auto TLCS900H::instructionXor(Target target, Source source) -> void {
  store(target, algorithmXor(load(target), load(source)));
}

template<typename Source, typename Offset>
auto TLCS900H::instructionXorCarry(Source source, Offset offset) -> void {
  if constexpr(Source::bits == 8 && is_same_v<Offset, Register<uint8>>) { if(load(offset).bit(3)) return (void)Undefined; }
  CF ^= load(source).bit(load(offset) & Source::bits - 1);
}