
/* WARNING: Unknown calling convention: __rustcall */
/* host_probe::modmaps::prepare */

void __rustcall
host_probe::modmaps::prepare
          (undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4,
          undefined4 param_5,undefined4 param_6)

{
  code *pcVar1;
  longlong lVar2;
  longlong lVar3;
  undefined8 *puVar4;
  undefined4 local_58;
  undefined4 uStack_54;
  undefined4 uStack_50;
  undefined4 uStack_4c;
  longlong local_48;
  int local_40 [2];
  undefined4 local_38;
  undefined4 uStack_34;
  undefined4 uStack_30;
  undefined4 uStack_2c;
  longlong local_28;
  
  prepare_files(local_40,param_1,param_2,param_3,param_4,param_5,param_6);
  if (local_40[0] == 1) {
    local_48 = local_28;
    local_58 = local_38;
    uStack_54 = uStack_34;
    uStack_50 = uStack_30;
    uStack_4c = uStack_2c;
    prepare::_closure_0_(&local_58);
                    /* WARNING: Does not return */
    pcVar1 = (code *)invalidInstructionException();
    (*pcVar1)();
  }
  local_48 = local_28;
  local_58 = local_38;
  uStack_54 = uStack_34;
  uStack_50 = uStack_30;
  uStack_4c = uStack_2c;
  if (DAT_14069acf0 != 0) {
    <std::sync::once_lock::OnceLock<alloc::vec::Vec<host_probe::modmaps::ModMap>>>
    ::initialize::
    <<std::sync::once_lock::OnceLock<alloc::vec::Vec<host_probe::modmaps::ModMap>>>::get_or_init<<std::sync::once_lock::OnceLock<alloc::vec::Vec<host_probe::modmaps::ModMap>>>::try_insert::_closure_0_>::_closure_0_,_>
              (MAPS,&local_58);
  }
  lVar2 = CONCAT44(uStack_54,local_58);
  if (lVar2 != -1) {
    if (local_48 != 0) {
      puVar4 = (undefined8 *)(CONCAT44(uStack_4c,uStack_50) + 0x20);
      lVar3 = local_48;
      do {
        if (puVar4[-4] != 0) {
          __rustc::__rust_dealloc(puVar4[-3],puVar4[-4],1);
          return;
        }
        if (puVar4[-1] != 0) {
          __rustc::__rust_dealloc(*puVar4,puVar4[-1],1);
          return;
        }
        puVar4 = puVar4 + 9;
        lVar3 = lVar3 + -1;
      } while (lVar3 != 0);
    }
    if (lVar2 != 0) {
      __rustc::__rust_dealloc(CONCAT44(uStack_4c,uStack_50),lVar2 * 0x48,8);
      return;
    }
  }
  return;
}


