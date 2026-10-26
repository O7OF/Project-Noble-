
/* WARNING: Unknown calling convention: __rustcall */
/* host_probe::fileshare::blf_file */

void __rustcall
host_probe::fileshare::blf_file
          (undefined8 *param_1,uint param_2,undefined8 param_3,undefined8 param_4)

{
  longlong local_b0 [2];
  longlong local_a0;
  ulonglong local_98;
  
  std::fs::read::inner(&local_a0,param_3,param_4);
  if (local_a0 != -1) {
                    /* WARNING: Could not emulate address calculation at 0x0001401683a0 */
                    /* WARNING: Treating indirect jump as call */
    (*(code *)((longlong)&UINT_1406be938 + (longlong)(int)(&UINT_1406be938)[(param_2 & 0xff) - 1]))
              ();
    return;
  }
  if ((1 < ((uint)local_98 & 3) - 2) && ((local_98 & 3) != 0)) {
    local_b0[0] = local_98 - 1;
    <core::io::error::CustomOwner_as_core::ops::drop::Drop>::drop
              (local_b0);
  }
  *param_1 = 0xffffffffffffffff;
  return;
}


