
/* WARNING: Unknown calling convention: __rustcall */
/* host_probe::modmaps::read_json */

void __rustcall
host_probe::modmaps::read_json
          (undefined4 *param_1,undefined8 param_2,undefined8 param_3)

{
  byte bVar1;
  longlong lVar2;
  ulonglong uVar3;
  byte *pbVar4;
  byte *pbVar5;
  uint uVar6;
  ulonglong local_a0;
  byte *local_98;
  code *local_90;
  ulonglong *local_88;
  code *local_80;
  undefined8 local_78;
  undefined8 local_70;
  undefined4 local_68;
  undefined4 uStack_64;
  undefined4 uStack_60;
  undefined4 uStack_5c;
  byte *local_58;
  undefined4 local_50;
  undefined4 uStack_4c;
  undefined4 uStack_48;
  undefined4 uStack_44;
  undefined8 local_40;
  undefined8 uStack_38;
  
  std::fs::read_to_string::inner(&local_68);
  lVar2 = CONCAT44(uStack_64,local_68);
  uVar3 = CONCAT44(uStack_5c,uStack_60);
  if (lVar2 == -1) {
    local_98 = (byte *)&local_78;
    local_90 = <std::path::Display_as_core::fmt::Display>::fmt;
    local_88 = &local_a0;
    local_80 = <core::io::error::Error_as_core::fmt::Display>::fmt
    ;
    local_a0 = uVar3;
    local_78 = param_2;
    local_70 = param_3;
    alloc::fmt::format::format_inner(&local_50,&DAT_1406a5738,&local_98);
    if ((1 < ((uint)local_a0 & 3) - 2) && ((local_a0 & 3) != 0)) {
      local_90 = (code *)(local_a0 - 1);
      local_98 = (byte *)CONCAT71(local_98._1_7_,3);
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop(&local_90);
    }
    *(ulonglong *)(param_1 + 2) = CONCAT44(uStack_4c,local_50);
    *(ulonglong *)(param_1 + 4) = CONCAT44(uStack_44,uStack_48);
    *(undefined8 *)(param_1 + 6) = local_40;
    *(undefined1 *)param_1 = 0xff;
  }
  else {
    pbVar4 = (byte *)0x0;
    do {
      while( true ) {
        local_98 = local_58;
        if (pbVar4 == local_58) goto LAB_14008dd44;
        pbVar5 = pbVar4 + uVar3;
        bVar1 = *pbVar5;
        local_98 = pbVar4;
        if ((-1 < (char)bVar1) || (bVar1 < 0xe0)) goto LAB_14008dd44;
        uVar6 = pbVar5[2] & 0x3f | (pbVar5[1] & 0x3f) << 6;
        if (0xef < bVar1) break;
        pbVar4 = pbVar5 + -uVar3 + 3;
        if ((uVar6 | (bVar1 & 0x1f) << 0xc) != 0xfeff) goto LAB_14008dd44;
      }
      pbVar4 = pbVar5 + -uVar3 + 4;
    } while ((pbVar5[3] & 0x3f | uVar6 << 6 | (bVar1 & 7) << 0x12) == 0xfeff);
LAB_14008dd44:
    local_90 = (code *)(local_58 + -(longlong)local_98);
    local_98 = local_98 + uVar3;
    local_88 = (ulonglong *)0x0;
    serde_json::de::from_trait::
    <serde_json::read::StrRead,serde_json::value::Value>
              (&local_50,&local_98);
    if ((char)local_50 == -1) {
      local_98 = (byte *)&local_78;
      local_90 = <std::path::Display_as_core::fmt::Display>::fmt;
      local_88 = &local_a0;
      local_80 = <serde_json::error::Error_as_core::fmt::Display>
                 ::fmt;
      local_78 = param_2;
      local_70 = param_3;
      alloc::fmt::format::format_inner(&local_68,&DAT_1406a574b,&local_98);
      core::ptr::drop_glue::<serde_json::error::Error>(&local_a0)
      ;
      *(byte **)(param_1 + 6) = local_58;
      param_1[2] = local_68;
      param_1[3] = uStack_64;
      param_1[4] = uStack_60;
      param_1[5] = uStack_5c;
      *(undefined1 *)param_1 = 0xff;
    }
    else {
      *(undefined8 *)(param_1 + 4) = local_40;
      *(undefined8 *)(param_1 + 6) = uStack_38;
      *param_1 = local_50;
      param_1[1] = uStack_4c;
      param_1[2] = uStack_48;
      param_1[3] = uStack_44;
    }
    if (lVar2 != 0) {
      __rustc::__rust_dealloc(uVar3,lVar2,1);
      return;
    }
  }
  return;
}


