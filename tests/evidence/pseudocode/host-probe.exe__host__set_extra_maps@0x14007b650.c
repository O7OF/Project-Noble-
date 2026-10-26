
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Unknown calling convention: __rustcall */
/* host_probe::host::set_extra_maps */

void __rustcall host_probe::host::set_extra_maps(undefined4 *param_1)

{
  code *pcVar1;
  longlong lVar2;
  undefined *puVar3;
  char cVar4;
  bool bVar5;
  undefined1 *local_38;
  byte local_30;
  
  LOCK();
  bVar5 = EXTRA_MAPS == '\0';
  if (bVar5) {
    EXTRA_MAPS = '\x01';
  }
  UNLOCK();
  if (!bVar5) {
    <std::sys::sync::mutex::futex::Mutex>::lock_contended(&EXTRA_MAPS);
  }
  puVar3 = __imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT;
  if ((*(ulonglong *)__imp__RNvNtNtCs59fwaEIuEkC_3std9panicking11panic_count18GLOBAL_PANIC_COUNT &
      0x7fffffffffffffff) == 0) {
    local_30 = 0;
  }
  else {
    local_30 = std::panicking::panic_count::is_zero_slow_path();
    local_30 = local_30 ^ 1;
  }
  if (DAT_1406999a1 != '\0') {
    local_38 = &EXTRA_MAPS;
    core::result::unwrap_failed
              ("called `Result::unwrap()` on an `Err` value",0x2b,&local_38,
               &anon_ee011d45c826cdbb3da69ebb3cc3f94a_131_llvm_15632039601783730204,
               &anon_ee011d45c826cdbb3da69ebb3cc3f94a_240_llvm_15632039601783730204);
                    /* WARNING: Does not return */
    pcVar1 = (code *)invalidInstructionException();
    (*pcVar1)();
  }
  lVar2 = CONCAT44(DAT_1406999a8._4_4_,(undefined4)DAT_1406999a8);
  if ((lVar2 != -1) && (lVar2 != 0)) {
    __rustc::__rust_dealloc
              (CONCAT44(DAT_1406999b0._4_4_,(undefined4)DAT_1406999b0),lVar2,1);
    return;
  }
  DAT_1406999a8._0_4_ = *param_1;
  DAT_1406999a8._4_4_ = param_1[1];
  DAT_1406999b0._0_4_ = param_1[2];
  DAT_1406999b0._4_4_ = param_1[3];
  _DAT_1406999b8 = param_1[4];
  uRam00000001406999bc = param_1[5];
  uRam00000001406999c0 = param_1[6];
  uRam00000001406999c4 = param_1[7];
  if (((local_30 == 0) && ((*(ulonglong *)puVar3 & 0x7fffffffffffffff) != 0)) &&
     (cVar4 = std::panicking::panic_count::is_zero_slow_path(), cVar4 == '\0')) {
    DAT_1406999a1 = '\x01';
  }
  cVar4 = EXTRA_MAPS;
  LOCK();
  EXTRA_MAPS = 0;
  UNLOCK();
  if (cVar4 == '\x02') {
    <std::sys::sync::mutex::futex::Mutex>::wake(&EXTRA_MAPS);
  }
  clear_file_cache();
  return;
}


