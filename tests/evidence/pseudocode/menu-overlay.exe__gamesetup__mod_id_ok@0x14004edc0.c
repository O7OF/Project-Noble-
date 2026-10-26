
/* WARNING: Unknown calling convention: __rustcall */
/* menu_overlay::gamesetup::mod_id_ok */

byte __rustcall
menu_overlay::gamesetup::mod_id_ok(char *param_1,ulonglong param_2)

{
  char cVar1;
  undefined1 auVar2 [16];
  ulonglong in_RAX;
  ulonglong uVar3;
  uint uVar4;
  longlong lVar5;
  bool bVar6;
  bool bVar7;
  
  if (0x14 < param_2) {
    return 0;
  }
  if (param_2 == 0) {
LAB_14004eec2:
    bVar6 = false;
  }
  else {
    in_RAX = 0;
    do {
      if ((byte)(param_1[in_RAX] - 0x3aU) < 0xf6) goto LAB_14004eec2;
      in_RAX = in_RAX + 1;
    } while (param_2 != in_RAX);
    cVar1 = *param_1;
    if (param_2 == 1) {
      bVar6 = false;
      if ((cVar1 == '+') || (bVar6 = false, cVar1 == '-')) goto LAB_14004eec5;
    }
    uVar3 = (ulonglong)(cVar1 == '+');
    if (param_2 - uVar3 < 0x11) {
      if (param_2 == uVar3) {
        bVar6 = true;
        in_RAX = 0;
      }
      else {
        in_RAX = 0;
        lVar5 = 0;
        do {
          bVar6 = (byte)param_1[lVar5 + uVar3] - 0x30 < 10;
          if (!bVar6) break;
          in_RAX = (ulonglong)((byte)param_1[lVar5 + uVar3] - 0x30) + in_RAX * 10;
          lVar5 = lVar5 + 1;
        } while (lVar5 != param_2 + -uVar3);
      }
    }
    else {
      in_RAX = 0;
      lVar5 = 0;
      do {
        auVar2._8_8_ = 0;
        auVar2._0_8_ = in_RAX;
        in_RAX = SUB168(auVar2 * ZEXT816(10),0);
        if (SUB168(auVar2 * ZEXT816(10),8) != 0) goto LAB_14004eec2;
        uVar4 = (byte)param_1[lVar5 + uVar3] - 0x30;
        bVar7 = CARRY8(in_RAX,(ulonglong)uVar4);
        in_RAX = in_RAX + uVar4;
        bVar6 = false;
        if ((9 < uVar4) || (bVar7)) break;
        lVar5 = lVar5 + 1;
        bVar6 = true;
      } while (lVar5 != param_2 + -uVar3);
    }
  }
LAB_14004eec5:
  return in_RAX != 0 & bVar6;
}


