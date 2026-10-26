
/* WARNING: Type propagation algorithm not settling */
/* reach_launcher::package::start */

ulonglong *
reach_launcher::package::start
          (ulonglong *param_1,void *param_2,size_t param_3,undefined *param_4,undefined8 param_5,
          longlong param_6,longlong param_7)

{
  byte *pbVar1;
  undefined1 auVar2 [16];
  bool bVar3;
  byte bVar4;
  byte bVar5;
  byte bVar6;
  byte bVar7;
  byte bVar8;
  byte bVar9;
  byte bVar10;
  byte bVar11;
  byte bVar12;
  byte bVar13;
  byte bVar14;
  byte bVar15;
  byte bVar16;
  byte bVar17;
  byte bVar18;
  byte bVar19;
  byte bVar20;
  byte bVar21;
  byte bVar22;
  byte bVar23;
  byte bVar24;
  byte bVar25;
  byte bVar26;
  code *pcVar27;
  undefined2 uVar28;
  undefined1 auVar29 [8];
  undefined8 *******pppppppuVar30;
  undefined1 uVar31;
  WORD WVar32;
  WORD WVar33;
  WORD WVar34;
  undefined6 uVar35;
  bool bVar36;
  char cVar37;
  undefined4 uVar38;
  int iVar39;
  uint uVar40;
  HANDLE hMutex;
  ulonglong uVar41;
  LPCWSTR pWVar42;
  undefined1 (*pauVar43) [16];
  undefined *puVar44;
  LPCWSTR *ppWVar45;
  byte *pbVar46;
  code *pcVar47;
  byte *pbVar48;
  LPCWSTR pWVar49;
  undefined4 *puVar50;
  LPCWSTR *ppWVar51;
  undefined8 *puVar52;
  longlong lVar53;
  undefined **ppuVar54;
  undefined8 *puVar55;
  undefined8 *puVar56;
  undefined8 *puVar57;
  ulonglong *puVar58;
  code *pcVar59;
  char *pcVar60;
  longlong lVar61;
  void *pvVar62;
  void *pvVar63;
  LPCWSTR *ppWVar64;
  code *pcVar65;
  ulonglong uVar66;
  byte *pbVar67;
  undefined *puVar68;
  ulonglong uVar69;
  LPCWSTR pWVar70;
  undefined8 *puVar71;
  undefined **s;
  SOCKET *pSVar72;
  undefined8 *******pppppppuVar73;
  undefined8 *******pppppppuVar74;
  undefined1 (*pauVar75) [16];
  DWORD dwTlsIndex;
  undefined1 *puVar76;
  longlong lVar77;
  undefined1 auVar78 [8];
  undefined8 uVar79;
  ushort uVar80;
  ulonglong uVar81;
  byte *pbVar82;
  ushort uVar83;
  ulonglong uVar84;
  uint uVar85;
  LPCWSTR *ppWVar86;
  longlong lVar87;
  longlong *plVar88;
  byte *pbVar89;
  undefined *puVar90;
  LPVOID pvVar91;
  size_t sVar92;
  undefined1 auVar93 [16];
  undefined1 auVar94 [16];
  byte bVar95;
  byte bVar96;
  byte bVar97;
  byte bVar98;
  byte bVar99;
  byte bVar100;
  byte bVar101;
  byte bVar102;
  byte bVar103;
  byte bVar104;
  byte bVar105;
  byte bVar106;
  byte bVar107;
  byte bVar108;
  byte bVar109;
  byte bVar110;
  byte bVar111;
  byte bVar112;
  byte bVar113;
  byte bVar114;
  byte bVar115;
  byte bVar116;
  byte bVar117;
  byte bVar118;
  byte bVar119;
  byte bVar120;
  byte bVar121;
  byte bVar122;
  byte bVar123;
  byte bVar124;
  byte bVar125;
  byte bVar126;
  byte bVar127;
  byte bVar128;
  byte bVar129;
  byte bVar130;
  byte bVar131;
  byte bVar132;
  byte bVar133;
  byte bVar134;
  undefined1 local_969;
  LPCWSTR *local_968;
  longlong local_960;
  undefined8 *local_958;
  code *local_950;
  LPCWSTR *local_948;
  LPCWSTR local_940;
  LPCWSTR *local_938;
  LPCWSTR local_930;
  LPCWSTR local_928;
  code *local_920;
  LPCWSTR *local_918;
  LPCWSTR *local_910;
  ulonglong local_908;
  undefined1 uStack_900;
  undefined7 uStack_8ff;
  undefined1 uStack_8f8;
  undefined7 uStack_8f7;
  undefined1 local_8e8 [10];
  undefined1 auStack_8de [2];
  undefined1 auStack_8dc [4];
  LPCWSTR *local_8d8;
  code *local_8d0;
  LPCWSTR *local_8c8;
  code *local_8c0;
  LPCWSTR *ppWStack_8b8;
  LPCWSTR *local_8b0;
  LPCWSTR *local_8a8;
  undefined8 *puStack_8a0;
  LPCWSTR *local_898;
  code *local_888;
  LPCWSTR *ppWStack_880;
  LPCWSTR *local_878;
  LPCWSTR local_870;
  ulonglong local_868;
  longlong local_860;
  ulonglong local_858;
  ulonglong *local_850;
  ulonglong local_848;
  undefined8 *local_840;
  ulonglong local_838;
  undefined1 *local_830;
  HANDLE local_828;
  size_t local_820;
  void *local_818;
  ulonglong local_810;
  undefined8 *local_808;
  ulonglong local_800;
  undefined1 *local_7f8;
  code *pcStack_7f0;
  LPCWSTR *local_7e8;
  undefined1 local_7d8 [8];
  undefined8 *******pppppppuStack_7d0;
  LPCWSTR *local_7c8;
  code *pcStack_7c0;
  LPCWSTR *local_7b8;
  code *pcStack_7b0;
  LPCWSTR *ppWStack_7a8;
  LPCWSTR *ppWStack_7a0;
  undefined1 *local_798;
  code *pcStack_790;
  LPCWSTR *local_788;
  LPCWSTR *ppWStack_780;
  undefined8 *puStack_778;
  LPCWSTR *ppWStack_770;
  undefined8 local_768;
  char *pcStack_760;
  undefined8 local_758;
  LPCWSTR local_750;
  undefined4 *puStack_748;
  LPCWSTR *local_740;
  undefined8 local_738;
  undefined8 *puStack_730;
  undefined8 local_728;
  undefined8 local_720;
  undefined8 uStack_718;
  undefined8 local_710;
  undefined8 local_568;
  ushort uStack_560;
  undefined6 uStack_55e;
  WORD WStack_558;
  ushort uStack_556;
  uint uStack_554;
  undefined8 uStack_550;
  undefined1 local_548;
  undefined7 uStack_547;
  code *pcStack_540;
  undefined1 local_538 [16];
  undefined1 *local_528;
  code *pcStack_520;
  LPCWSTR *local_518;
  LPCWSTR *ppWStack_510;
  undefined8 *local_508;
  LPCWSTR *ppWStack_500;
  char *local_4f8;
  undefined8 local_4f0;
  ulonglong *local_4e8;
  undefined *local_4e0;
  undefined8 local_4d8;
  ulonglong *local_4d0;
  size_t local_2f8;
  LPCWSTR *local_2f0;
  LPCWSTR *local_2e8;
  undefined *local_2e0;
  ulonglong local_2d8;
  ulonglong *local_2d0;
  ulonglong local_2c8;
  undefined4 *local_2c0;
  LPCWSTR local_2b8;
  undefined8 *local_2b0;
  LPCWSTR local_2a8;
  undefined *local_2a0;
  undefined8 local_298;
  ulonglong local_290;
  ulonglong *local_288;
  longlong local_280;
  void *local_278;
  LPCWSTR local_270;
  undefined8 *local_268;
  longlong local_260;
  longlong local_258;
  LPCWSTR local_250;
  LPCWSTR *ppWStack_248;
  LPCWSTR *local_238;
  LPCWSTR local_230;
  undefined8 *puStack_228;
  LPCWSTR local_220;
  LPCWSTR *ppWStack_218;
  size_t local_210;
  code *local_200;
  LPCWSTR *local_1f8;
  LPCWSTR *local_1f0;
  LPCWSTR *local_1e8;
  longlong local_1e0;
  LPCWSTR *local_1d8;
  LPCWSTR *local_1d0;
  undefined *local_1c0;
  undefined1 *local_1b8;
  size_t local_1b0;
  longlong local_1a0;
  LPVOID local_198;
  undefined *local_190;
  LPCWSTR local_180;
  undefined4 *puStack_178;
  LPCWSTR *local_170;
  longlong local_168;
  code *local_160;
  HANDLE local_158;
  longlong local_148;
  LPCWSTR local_140;
  ulonglong local_138;
  LPCWSTR *local_128;
  LPCWSTR *local_120;
  HANDLE local_118;
  undefined8 local_110;
  undefined8 *local_108;
  undefined8 local_100;
  undefined1 *local_f8;
  undefined8 *local_f0;
  undefined1 *local_e8;
  ulonglong local_e0;
  longlong local_d8;
  longlong local_d0;
  ulonglong local_c8;
  undefined *local_c0;
  undefined *local_b8;
  ulonglong local_b0;
  undefined8 *******local_a8;
  undefined1 *local_a0;
  int local_98 [2];
  ulonglong local_90;
  
  local_820 = param_3;
  local_818 = param_2;
  local_2e0 = param_4;
  hMutex = running::lock_named(0x14015b151,0x1c,30000);
  local_118 = hMutex;
  uVar41 = running::settle();
  if ((char)uVar41 != '\0') {
    *param_1 = 0xffffffffffffffff;
    goto LAB_140015503;
  }
  std::sys::env::windows::getenv(&local_568,0x14015a463,0xc);
  lVar77 = CONCAT26(local_568._6_2_,
                    CONCAT24(local_568._4_2_,
                             CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))));
  if (lVar77 == -1) {
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    puVar71 = (undefined8 *)__rustc::__rust_alloc(0x18,1);
    if (puVar71 == (undefined8 *)0x0) {
      alloc::raw_vec::handle_error(1,0x18);
      goto LAB_14001be41;
    }
    *puVar71 = 0x5050414c41434f4c;
    puVar71[1] = 0x2073692041544144;
    puVar71[2] = 0x2e74657320746f6e;
    *param_1 = 0x18;
    param_1[1] = (ulonglong)puVar71;
    param_1[2] = 0x18;
    goto LAB_140015503;
  }
  local_278 = (void *)CONCAT62(uStack_55e,uStack_560);
  <std::path::Path>::_join
            (&local_1e0,local_278,CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558)),
             (undefined8 *******)&DAT_14015a487,&DAT_00000010);
  ppWVar64 = local_1d0;
  ppWVar51 = local_1d8;
  local_960 = lVar77;
  <std::path::Path>::_join
            (&local_148,local_1d8,(size_t)local_1d0,(undefined8 *******)&DAT_14015a497,&DAT_00000004
            );
  pWVar49 = local_140;
  local_568._0_1_ = '\x01';
  pWVar42 = <std::fs::DirBuilder>::_create((char *)&local_568,local_140,local_138)
  ;
  if (((pWVar42 != (LPCWSTR)0x0) && (1 < ((uint)pWVar42 & 3) - 2)) &&
     (((ulonglong)pWVar42 & 3) != 0)) {
    uStack_560 = (ushort)((longlong)pWVar42 + -1);
    uStack_55e = (undefined6)((ulonglong)((longlong)pWVar42 + -1) >> 0x10);
    local_568._0_1_ = '\x03';
    <core::io::error::CustomOwner_as_core::ops::drop::Drop>::drop
              ((longlong *)&uStack_560);
  }
  std::sys::fs::read_dir(&local_568,pWVar49,local_138);
  uVar35 = uStack_55e;
  uVar83 = uStack_560;
  WVar34 = local_568._6_2_;
  WVar33 = local_568._4_2_;
  WVar32 = local_568._2_2_;
  uVar28 = CONCAT11(local_568._1_1_,(char)local_568);
  local_850 = param_1;
  local_828 = hMutex;
  if (CONCAT26(local_568._6_2_,CONCAT24(local_568._4_2_,CONCAT22(local_568._2_2_,uVar28))) == 2) {
    if ((1 < ((uint)CONCAT62(uStack_55e,uStack_560) & 3) - 2) && ((uStack_560 & 3) != 0)) {
      lVar77 = CONCAT62(uStack_55e,uStack_560) + -1;
      uStack_560 = (ushort)lVar77;
      uStack_55e = (undefined6)((ulonglong)lVar77 >> 0x10);
      local_568._0_1_ = '\x03';
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop((longlong *)&uStack_560);
    }
  }
  else {
    memcpy(&local_7c8,&WStack_558,0x260);
    pppppppuStack_7d0._0_2_ = uVar83;
    pppppppuStack_7d0._2_2_ = (short)uVar35;
    pppppppuStack_7d0._4_2_ = (short)((uint6)uVar35 >> 0x10);
    pppppppuStack_7d0._6_2_ = (short)((uint6)uVar35 >> 0x20);
    local_7d8._0_2_ = uVar28;
    local_7d8._2_2_ = WVar32;
    local_7d8._4_2_ = WVar33;
    local_7d8._6_2_ = WVar34;
    <alloc::vec::Vec<std::path::PathBuf>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<std::path::PathBuf,core::iter::adapters::filter::Filter<core::iter::adapters::map::Map<core::iter::adapters::filter_map::FilterMap<std::fs::ReadDir,reach_launcher::package::prune_runs::{closure#0}>,reach_launcher::package::prune_runs::{closure#1}>,reach_launcher::package::prune_runs::{closure#2}>>>
    ::from_iter(&local_568,(longlong *)local_7d8);
    puVar71 = (undefined8 *)CONCAT62(uStack_55e,uStack_560);
    uVar41 = CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
    if (1 < uVar41) {
      puVar52 = puVar71;
      if (uVar41 < 0x15) {
        lVar77 = 0x20;
        do {
          core::slice::sort::shared::smallsort::insert_tail::
          <std::path::PathBuf,<std::path::PathBuf_as_core::cmp::PartialOrd>::lt>
                    (puVar71,(undefined8 *)((longlong)puVar71 + lVar77));
          lVar77 = lVar77 + 0x20;
        } while (uVar41 << 5 != lVar77);
        lVar77 = 0;
        if (uVar41 >= 10) {
          lVar77 = uVar41 - 10;
        }
        if (uVar41 < 10 || uVar41 - 10 == 0) goto LAB_140015220;
        puVar55 = puVar71 + lVar77 * 4;
      }
      else {
        core::slice::sort::stable::driftsort_main::
        <std::path::PathBuf,<std::path::PathBuf_as_core::cmp::PartialOrd>::lt,alloc::vec::Vec<std::path::PathBuf>>
                  (puVar71,uVar41,&local_969);
        puVar55 = puVar71 + uVar41 * 4 + -0x28;
      }
      do {
        pWVar42 = std::sys::fs::remove_dir_all(puVar52[1],puVar52[2]);
        if (((pWVar42 != (LPCWSTR)0x0) && (1 < ((uint)pWVar42 & 3) - 2)) &&
           (((ulonglong)pWVar42 & 3) != 0)) {
          pppppppuStack_7d0 = (undefined8 *******)((longlong)pWVar42 + -1);
          local_7d8[0] = 3;
          <core::io::error::CustomOwner_as_core::ops::drop::Drop>
          ::drop((longlong *)(local_7d8 + 8));
        }
        puVar52 = puVar52 + 4;
      } while (puVar52 != puVar55);
    }
LAB_140015220:
    if (uVar41 != 0) {
      puVar52 = puVar71 + 1;
      do {
        if (puVar52[-1] != 0) {
          __rustc::__rust_dealloc((LPVOID)*puVar52,puVar52[-1],1);
        }
        puVar52 = puVar52 + 4;
        uVar41 = uVar41 - 1;
      } while (uVar41 != 0);
    }
    lVar77 = CONCAT26(local_568._6_2_,
                      CONCAT24(local_568._4_2_,
                               CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))))
    ;
    if (lVar77 != 0) {
      __rustc::__rust_dealloc(puVar71,lVar77 << 5,8);
    }
  }
  local_8e8._0_2_ = 0;
  local_8e8._2_2_ = 0;
  local_8e8._4_2_ = 0;
  local_8e8._6_2_ = 0;
  local_8e8._8_2_ = 0;
  auStack_8de = (undefined1  [2])0x0;
  auStack_8dc._0_2_ = 0;
  auStack_8dc._2_2_ = 0;
  GetLocalTime((LPSYSTEMTIME)local_8e8);
  puVar76 = local_8e8 + 2;
  local_528 = local_8e8 + 10;
  local_518 = (LPCWSTR *)(local_8e8 + 0xc);
  local_568._0_1_ = (char)(LPSYSTEMTIME)local_8e8;
  cVar37 = (char)local_568;
  local_568._1_1_ = (undefined1)((ulonglong)local_8e8 >> 8);
  uVar31 = local_568._1_1_;
  local_568._2_2_ = (WORD)((ulonglong)local_8e8 >> 0x10);
  WVar32 = local_568._2_2_;
  local_568._4_2_ = (WORD)((ulonglong)local_8e8 >> 0x20);
  WVar33 = local_568._4_2_;
  local_568._6_2_ = (WORD)((ulonglong)local_8e8 >> 0x30);
  WVar34 = local_568._6_2_;
  uStack_560 = 0xd230;
  uStack_55e = 0x14014;
  WStack_558 = (WORD)puVar76;
  uStack_556 = (ushort)((ulonglong)puVar76 >> 0x10);
  uStack_554 = (uint)((ulonglong)puVar76 >> 0x20);
  uStack_550._0_1_ = 0x30;
  uStack_550._1_1_ = 0xd2;
  uStack_550._2_1_ = 0x14;
  uStack_550._3_1_ = 0x40;
  uStack_550._4_4_ = 1;
  local_548 = SUB81(local_8e8 + 6,0);
  uStack_547 = (undefined7)((ulonglong)(local_8e8 + 6) >> 8);
  pcStack_540 = <u16_as_core::fmt::Display>::fmt;
  local_538._8_8_ = <u16_as_core::fmt::Display>::fmt;
  local_538._0_8_ = local_8e8 + 8;
  pcStack_520 = <u16_as_core::fmt::Display>::fmt;
  ppWStack_510 = (LPCWSTR *)<u16_as_core::fmt::Display>::fmt;
  alloc::fmt::format::format_inner
            ((ulonglong *)local_7d8,(ushort *)&DAT_14015aca9,(ulonglong)&local_568);
  pppppppuVar30 = pppppppuStack_7d0;
  <std::path::Path>::_join
            (&local_258,pWVar49,local_138,pppppppuStack_7d0,(undefined1 *)local_7c8);
  if (local_7d8 != (undefined1  [8])0x0) {
    __rustc::__rust_dealloc(pppppppuVar30,local_7d8,1);
  }
  ppWVar45 = ppWStack_248;
  pWVar49 = local_250;
  local_568._0_1_ = '\x01';
  pWVar42 = <std::fs::DirBuilder>::_create
                      ((char *)&local_568,local_250,(ulonglong)ppWStack_248);
  hMutex = local_828;
  param_1 = local_850;
  if (pWVar42 == (LPCWSTR)0x0) {
    <std::path::Path>::_join
              (&local_1a0,ppWVar51,(size_t)ppWVar64,(undefined8 *******)&DAT_14015a49b,&DAT_0000000d
              );
    puVar90 = local_190;
    pvVar91 = local_198;
    puVar68 = local_190;
    bVar36 = <std::path::Path>::is_file((longlong)local_198,(longlong)local_190);
    puVar44 = 
    __imp__RNvNCNvNvMNtNtCs59fwaEIuEkC_3std4hash6randomNtB8_11RandomState3new4KEYS023___RUST_STD_INTERNAL_VAL
    ;
    uVar38 = SUB84(puVar68,0);
    if (bVar36) {
LAB_140015a4e:
      std::fs::read_to_string::inner(&local_568,(longlong)pvVar91,puVar90);
      lVar77 = CONCAT26(local_568._6_2_,
                        CONCAT24(local_568._4_2_,
                                 CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))
                                ));
      if (lVar77 == -1) {
        if ((1 < ((uint)CONCAT62(uStack_55e,uStack_560) & 3) - 2) && ((uStack_560 & 3) != 0)) {
          pppppppuStack_7d0 = (undefined8 *******)(CONCAT62(uStack_55e,uStack_560) + -1);
          local_7d8[0] = 3;
          <core::io::error::CustomOwner_as_core::ops::drop::Drop>
          ::drop((longlong *)(local_7d8 + 8));
        }
        pWVar49 = (LPCWSTR)0x1;
        ppWVar51 = (LPCWSTR *)0x0;
        lVar77 = 0;
      }
      else {
        pWVar49 = (LPCWSTR)CONCAT62(uStack_55e,uStack_560);
        ppWVar51 = (LPCWSTR *)CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
      }
      ppWVar64 = (LPCWSTR *)0x0;
      do {
        pbVar82 = (byte *)((longlong)ppWVar51 - (longlong)ppWVar64);
        pbVar46 = (byte *)((longlong)pWVar49 + (longlong)ppWVar64);
        pWVar42 = pWVar49;
        if (&DAT_0000000f < pbVar82) {
          pbVar67 = pbVar46;
          uVar41 = core::slice::memchr::memchr_aligned
                             (10,(longlong)pbVar46,(ulonglong)pbVar82);
          if ((uVar41 & 1) == 0) break;
        }
        else {
          if (ppWVar51 == ppWVar64) break;
          pbVar67 = (byte *)0x0;
          while (pbVar46[(longlong)pbVar67] != 10) {
            pbVar67 = pbVar67 + 1;
            if (pbVar82 == pbVar67) goto LAB_140015b32;
          }
        }
        ppWVar45 = (LPCWSTR *)(pbVar67 + (longlong)ppWVar64 + 1);
        if (((LPCWSTR *)((longlong)ppWVar64 + (longlong)pbVar67) < ppWVar51) &&
           (pbVar46[(longlong)pbVar67] == 10)) {
          ppWVar51 = ppWVar45;
          if (ppWVar45 == (LPCWSTR *)0x0) goto LAB_140015b69;
          goto LAB_140015b3d;
        }
        ppWVar64 = ppWVar45;
      } while (ppWVar45 <= ppWVar51);
LAB_140015b32:
      if (ppWVar51 == (LPCWSTR *)0x0) {
        pWVar42 = (LPCWSTR)0x1;
LAB_140015b67:
        ppWVar45 = (LPCWSTR *)0x0;
      }
      else {
LAB_140015b3d:
        ppWVar45 = ppWVar51;
        if (((byte *)((longlong)pWVar49 + -1))[(longlong)ppWVar51] == 10) {
          ppWVar45 = (LPCWSTR *)((longlong)ppWVar51 + -1);
          if (ppWVar45 == (LPCWSTR *)0x0) goto LAB_140015b67;
          if (*(byte *)((longlong)(pWVar49 + -1) + (longlong)ppWVar51) == 0xd) {
            ppWVar45 = (LPCWSTR *)((longlong)ppWVar51 + -2);
          }
        }
      }
LAB_140015b69:
      pbVar46 = <str>::trim_matches::<<char>::is_whitespace>((byte *)pWVar42,(longlong)ppWVar45);
      local_2e8 = ppWVar45;
      if ((longlong)ppWVar45 < 0) {
        lVar87 = 0;
LAB_140015b86:
        alloc::raw_vec::handle_error(lVar87,ppWVar45);
        goto LAB_14001be41;
      }
      if (ppWVar45 == (LPCWSTR *)0x0) {
        pcVar47 = (code *)0x1;
      }
      else {
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        lVar87 = 1;
        pcVar47 = (code *)__rustc::__rust_alloc((longlong)ppWVar45,1);
        if (pcVar47 == (code *)0x0) goto LAB_140015b86;
        memcpy(pcVar47,pbVar46,(size_t)ppWVar45);
      }
      sVar92 = local_820;
      if (lVar77 != 0) {
        __rustc::__rust_dealloc(pWVar49,lVar77,1);
      }
      ppWVar64 = local_1d0;
      ppWVar51 = local_1d8;
      <std::path::Path>::_join
                (&local_168,local_1d8,(size_t)local_1d0,(undefined8 *******)"settings.txt",
                 &DAT_0000000c);
      std::fs::read_to_string::inner(&local_568,(longlong)local_160,local_158);
      pWVar42 = (LPCWSTR)CONCAT26(local_568._6_2_,
                                  CONCAT24(local_568._4_2_,
                                           CONCAT22(local_568._2_2_,
                                                    CONCAT11(local_568._1_1_,(char)local_568))));
      local_950 = pcVar47;
      if (pWVar42 != (LPCWSTR)0xffffffffffffffff) {
        pvVar91 = (LPVOID)CONCAT62(uStack_55e,uStack_560);
        pbVar46 = (byte *)CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
        pbVar82 = (byte *)0x0;
        local_968 = ppWVar51;
        local_928 = pWVar42;
        local_910 = ppWVar64;
        pbVar67 = (byte *)0x0;
joined_r0x000140015c9f:
        do {
          pbVar89 = pbVar82;
          if (pbVar46 < pbVar82) {
LAB_140015d22:
            bVar36 = true;
            pbVar48 = pbVar67;
            pbVar82 = pbVar46;
            if (pbVar46 == pbVar67) break;
          }
          else {
            pWVar42 = (LPCWSTR)(pbVar46 + -(longlong)pbVar82);
            pWVar49 = (LPCWSTR)((longlong)pvVar91 + (longlong)pbVar82);
            pbVar89 = pbVar46;
            if (pWVar42 <= &DAT_0000000f) {
              if (pbVar46 != pbVar82) {
                pWVar70 = (LPCWSTR)0x0;
                do {
                  if (*(byte *)((longlong)pWVar49 + (longlong)pWVar70) == 10) goto LAB_140015cfe;
                  pWVar70 = (LPCWSTR)((longlong)pWVar70 + 1);
                } while (pWVar42 != pWVar70);
              }
              goto LAB_140015d22;
            }
            pWVar70 = pWVar49;
            uVar41 = core::slice::memchr::memchr_aligned
                               (10,(longlong)pWVar49,(ulonglong)pWVar42);
            if ((uVar41 & 1) == 0) goto LAB_140015d22;
LAB_140015cfe:
            pbVar48 = (byte *)((longlong)pWVar70 + (longlong)pbVar82) + 1;
            pbVar89 = pbVar82 + (longlong)pWVar70;
            pbVar82 = pbVar48;
            if ((pbVar46 <= pbVar89) || (*(byte *)((longlong)pWVar49 + (longlong)pWVar70) != 10))
            goto joined_r0x000140015c9f;
            bVar36 = false;
            pbVar89 = pbVar48;
          }
          pcVar47 = local_950;
          pbVar1 = (byte *)((longlong)pvVar91 + (longlong)pbVar67);
          uVar66 = (longlong)pbVar82 - (longlong)pbVar67;
          uVar41 = uVar66;
          if ((uVar66 != 0) && (pbVar82[(longlong)pvVar91 + -1] == 10)) {
            uVar41 = uVar66 - 1;
            if (uVar41 == 0) {
              uVar41 = 0;
            }
            else if (pbVar82[(longlong)pvVar91 + -2] == 0xd) {
              uVar41 = uVar66 - 2;
            }
          }
          if ((4 < uVar41) && (pbVar1[4] == 0x3d && *(int *)pbVar1 == 0x656d616e)) {
            puVar71 = (undefined8 *)(uVar41 - 5);
            pWVar42 = (LPCWSTR)<str>::trim_matches::<<char>::is_whitespace>
                                         (pbVar1 + 5,(longlong)puVar71);
            sVar92 = local_820;
            ppWVar64 = local_910;
            ppWVar45 = (LPCWSTR *)0xffffffffffffffff;
            pWVar49 = pWVar42;
            if (pWVar42 == (LPCWSTR)0x0) goto LAB_140015e04;
            local_958 = puVar71;
            if (&DAT_0000001f < puVar71) {
              lVar77 = core::str::count::do_count_chars
                                 ((ulonglong)pWVar42,(ulonglong)puVar71);
            }
            else {
              lVar77 = core::str::count::char_count_general_case
                                 ((longlong)pWVar42,(ulonglong)puVar71);
            }
            if (((0xe < lVar77 - 1U) ||
                (puVar71 = local_958,
                pbVar46 = <str>::trim_matches::<<char>::is_whitespace>
                                    ((byte *)pWVar42,(longlong)local_958), puVar71 != local_958)) ||
               (iVar39 = memcmp(pbVar46,pWVar42,(size_t)local_958), iVar39 != 0))
            goto LAB_140015e04;
            goto LAB_14001b05d;
          }
          pbVar82 = pbVar89;
          pbVar67 = pbVar48;
        } while (!bVar36);
        ppWVar45 = (LPCWSTR *)0xffffffffffffffff;
        pcVar47 = local_950;
        ppWVar64 = local_910;
        sVar92 = local_820;
        goto LAB_140015e04;
      }
      if ((1 < ((uint)CONCAT62(uStack_55e,uStack_560) & 3) - 2) && ((uStack_560 & 3) != 0)) {
        pppppppuStack_7d0 = (undefined8 *******)(CONCAT62(uStack_55e,uStack_560) + -1);
        local_7d8[0] = 3;
        <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
        drop((longlong *)(local_7d8 + 8));
      }
      goto LAB_140015ef2;
    }
    if (*(longlong *)
         __imp__RNvNCNvNvMNtNtCs59fwaEIuEkC_3std4hash6randomNtB8_11RandomState3new4KEYS023___RUST_STD_INTERNAL_VAL
        != 0) {
      std::sys::thread_local::guard::windows::enable();
    }
    if (*(int *)(puVar44 + 0x18) == 0) {
      dwTlsIndex = <std::sys::thread_local::key::windows::LazyKey>::init
                             ((longlong *)
                              __imp__RNvNCNvNvMNtNtCs59fwaEIuEkC_3std4hash6randomNtB8_11RandomState3new4KEYS023___RUST_STD_INTERNAL_VAL
                             );
      pauVar43 = TlsGetValue(dwTlsIndex);
    }
    else {
      dwTlsIndex = *(int *)(puVar44 + 0x18) - 1;
      pauVar43 = TlsGetValue(dwTlsIndex);
    }
    if (pauVar43 < (undefined1 (*) [16])0x2) {
      pauVar75 = pauVar43;
                    /* WARNING: Read-only address (ram,0x00014015a2d0) is written */
                    /* WARNING: Read-only address (ram,0x00014015a2e0) is written */
      pauVar43 = (undefined1 (*) [16])
                 <std::sys::thread_local::os::Storage<core::cell::Cell<(u64,u64)>,8usize>>
                 ::try_initialize::
                 <<std::hash::random::RandomState>::new::KEYS::__rust_std_internal_init_fn>
                           (dwTlsIndex,(longlong)pauVar43,(byte *)0x0);
      uVar38 = SUB84(pauVar75,0);
    }
    if (pauVar43 == (undefined1 (*) [16])0x0) {
      std::thread::local::panic_access_error
                (&anon_0ba670aeb02bc321804ac4e2f5d0109e_1_llvm_15297295612839780106);
      goto LAB_14001be41;
    }
    auVar2 = *pauVar43;
    *(longlong *)*pauVar43 = *(longlong *)*pauVar43 + 1;
    auVar93._0_8_ = auVar2._0_8_;
    auVar93._8_4_ = auVar2._0_4_;
    auVar93._12_4_ = auVar2._4_4_;
    auVar93 = auVar93 ^ (undefined1  [16])::_rdata;
    local_568._0_1_ = auVar93[0];
    local_568._1_1_ = auVar93[1];
    local_568._2_2_ = auVar93._2_2_;
    local_568._4_2_ = auVar93._4_2_;
    local_568._6_2_ = auVar93._6_2_;
    uStack_560 = auVar93._8_2_;
    uStack_55e = auVar93._10_6_;
    auVar94._0_8_ = auVar2._8_8_;
    auVar94._8_4_ = auVar2._8_4_;
    auVar94._12_4_ = auVar2._12_4_;
    auVar93 = auVar94 ^ (undefined1  [16])s_modnarodsetybdet_14015a2e0;
    WStack_558 = auVar93._0_2_;
    uStack_556 = auVar93._2_2_;
    uStack_554 = auVar93._4_4_;
    uStack_550._0_1_ = auVar93[8];
    uStack_550._1_1_ = auVar93[9];
    uStack_550._2_1_ = auVar93[10];
    uStack_550._3_1_ = auVar93[0xb];
    uStack_550._4_4_ = auVar93._12_4_;
    local_548 = auVar2[0];
    uStack_547 = auVar2._1_7_;
    local_538 = (undefined1  [16])0x0;
    local_528 = (undefined1 *)0x0;
    pcStack_540 = auVar94._0_8_;
    uVar41 = <std::time::SystemTime>::now();
    local_8e8._4_4_ = uVar38;
    local_8e8._0_4_ = (int)uVar41;
    <std::time::SystemTime>::duration_since
              ((ulonglong *)local_7d8,(longlong *)local_8e8,0xd53e8000,0x19db1de);
    auVar2 = ZEXT816(0);
    if (local_7d8[0] == '\0') {
      auVar2._8_8_ = 0;
      auVar2._0_8_ = pppppppuStack_7d0;
      auVar2 = ZEXT816(1000000000) * auVar2 + ZEXT416((uint)local_7c8);
    }
    _local_7d8 = (_SYSTEMTIME)auVar2;
    <core::hash::sip::Hasher<core::hash::sip::Sip13Rounds>as_core::hash::Hasher>
    ::write(&local_568,(uint *)local_7d8,0x10);
    uVar38 = std::process::id();
    local_7d8._0_2_ = (short)uVar38;
    local_7d8._2_2_ = (short)((uint)uVar38 >> 0x10);
    <core::hash::sip::Hasher<core::hash::sip::Sip13Rounds>as_core::hash::Hasher>
    ::write(&local_568,(uint *)local_7d8,4);
    lVar77 = CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
    uVar84 = local_538._0_8_ << 0x38 | local_538._8_8_;
    uVar66 = CONCAT44(uStack_550._4_4_,
                      CONCAT13(uStack_550._3_1_,
                               CONCAT12(uStack_550._2_1_,
                                        CONCAT11(uStack_550._1_1_,(undefined1)uStack_550)))) ^
             uVar84;
    uVar81 = CONCAT26(local_568._6_2_,
                      CONCAT24(local_568._4_2_,
                               CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))))
             + lVar77;
    uVar69 = CONCAT62(uStack_55e,uStack_560) + uVar66;
    uVar41 = (lVar77 << 0xd | (ulonglong)(uStack_554 >> 0x13)) ^ uVar81;
    uVar66 = (uVar66 << 0x10 | uVar66 >> 0x30) ^ uVar69;
    uVar69 = uVar69 + uVar41;
    uVar81 = (uVar81 << 0x20 | uVar81 >> 0x20) + uVar66;
    uVar41 = (uVar41 << 0x11 | uVar41 >> 0x2f) ^ uVar69;
    uVar66 = (uVar66 << 0x15 | uVar66 >> 0x2b) ^ uVar81;
    uVar81 = (uVar81 ^ uVar84) + uVar41;
    uVar69 = ((uVar69 << 0x20 | uVar69 >> 0x20) ^ 0xff) + uVar66;
    uVar41 = (uVar41 << 0xd | uVar41 >> 0x33) ^ uVar81;
    uVar66 = (uVar66 << 0x10 | uVar66 >> 0x30) ^ uVar69;
    uVar69 = uVar69 + uVar41;
    uVar81 = (uVar81 << 0x20 | uVar81 >> 0x20) + uVar66;
    uVar41 = (uVar41 << 0x11 | uVar41 >> 0x2f) ^ uVar69;
    uVar66 = (uVar66 << 0x15 | uVar66 >> 0x2b) ^ uVar81;
    uVar81 = uVar81 + uVar41;
    uVar69 = (uVar69 << 0x20 | uVar69 >> 0x20) + uVar66;
    uVar41 = (uVar41 << 0xd | uVar41 >> 0x33) ^ uVar81;
    uVar66 = (uVar66 << 0x10 | uVar66 >> 0x30) ^ uVar69;
    uVar69 = uVar69 + uVar41;
    uVar81 = (uVar81 << 0x20 | uVar81 >> 0x20) + uVar66;
    uVar41 = (uVar41 << 0x11 | uVar41 >> 0x2f) ^ uVar69;
    uVar66 = (uVar66 << 0x15 | uVar66 >> 0x2b) ^ uVar81;
    uVar69 = (uVar69 << 0x20 | uVar69 >> 0x20) + uVar66;
    uVar41 = (uVar41 << 0xd | uVar41 >> 0x33) ^ uVar81 + uVar41;
    uVar66 = (uVar66 << 0x10 | uVar66 >> 0x30) ^ uVar69;
    uVar69 = uVar69 + uVar41;
    uVar41 = ((uVar69 << 0x20 | uVar69 >> 0x20) ^ (uVar41 << 0x11 | uVar41 >> 0x2f) ^
              (uVar66 << 0x15 | uVar66 >> 0x2b) ^ uVar69) & 0xffffffffffff;
    local_908._0_1_ = (byte)uVar41;
    local_908._1_7_ = (undefined7)(uVar41 >> 8);
    stack0xfffffffffffff720 = <usize_as_core::fmt::LowerHex>::fmt;
    local_8e8._0_8_ = &local_908;
    alloc::fmt::format::format_inner
              ((ulonglong *)local_7d8,(ushort *)&DAT_14015a433,(ulonglong)local_8e8);
    local_568._0_1_ = (char)local_7d8;
    local_568._1_1_ = (undefined1)((ulonglong)local_7d8 >> 8);
    local_568._2_2_ = (WORD)((ulonglong)local_7d8 >> 0x10);
    local_568._4_2_ = (WORD)((ulonglong)local_7d8 >> 0x20);
    local_568._6_2_ = (WORD)((ulonglong)local_7d8 >> 0x30);
    uStack_560 = 0xdbb0;
    uStack_55e = 0x14001;
    alloc::fmt::format::format_inner
              (&local_c8,(ushort *)&DAT_14015a4a8,(ulonglong)&local_568);
    if (local_7d8 != (undefined1  [8])0x0) {
      __rustc::__rust_dealloc(pppppppuStack_7d0,local_7d8,1);
    }
    pvVar91 = local_198;
    puVar44 = std::fs::write::inner
                        ((longlong)local_198,local_190,local_c0,local_b8);
    if (local_c8 != 0) {
      __rustc::__rust_dealloc(local_c0,local_c8,1);
    }
    puVar90 = local_190;
    if (puVar44 == (undefined *)0x0) goto LAB_140015a4e;
    local_568._0_1_ = (char)local_8e8;
    cVar37 = (char)local_568;
    local_568._1_1_ = (undefined1)((ulonglong)local_8e8 >> 8);
    uVar31 = local_568._1_1_;
    local_568._2_2_ = (WORD)((ulonglong)local_8e8 >> 0x10);
    WVar32 = local_568._2_2_;
    local_568._4_2_ = (WORD)((ulonglong)local_8e8 >> 0x20);
    WVar33 = local_568._4_2_;
    local_568._6_2_ = (WORD)((ulonglong)local_8e8 >> 0x30);
    WVar34 = local_568._6_2_;
    uStack_560 = 0xe0;
    uStack_55e = 0x14010;
    WStack_558 = (WORD)&local_1c0;
    uStack_556 = (ushort)((ulonglong)&local_1c0 >> 0x10);
    uStack_554 = (uint)((ulonglong)&local_1c0 >> 0x20);
    uStack_550._0_1_ = 0;
    uStack_550._1_1_ = 0xcf;
    uStack_550._2_1_ = 0x14;
    uStack_550._3_1_ = 0x40;
    uStack_550._4_4_ = 1;
    local_1c0 = puVar44;
    alloc::fmt::format::format_inner
              ((ulonglong *)local_7d8,(ushort *)&DAT_14015a39e,(ulonglong)&local_568);
    _local_8e8 = _local_7d8;
    local_8d8 = local_7c8;
    local_220 = local_250;
    ppWStack_218 = ppWStack_248;
    uStack_560 = 0xdbb0;
    uStack_55e = 0x14001;
    WStack_558 = (WORD)&local_220;
    uStack_556 = (ushort)((ulonglong)&local_220 >> 0x10);
    uStack_554 = (uint)((ulonglong)&local_220 >> 0x20);
    uStack_550._0_1_ = 0xe0;
    uStack_550._1_1_ = 0;
    uStack_550._2_1_ = 0x10;
    uStack_550._3_1_ = 0x40;
    uStack_550._4_4_ = 1;
    local_568._0_1_ = cVar37;
    local_568._1_1_ = uVar31;
    local_568._2_2_ = WVar32;
    local_568._4_2_ = WVar33;
    local_568._6_2_ = WVar34;
    alloc::fmt::format::format_inner
              (&local_908,(ushort *)&DAT_14015a3d7,(ulonglong)&local_568);
    hMutex = local_828;
    param_1 = local_850;
    if (local_8e8._0_8_ != 0) {
      __rustc::__rust_dealloc(stack0xfffffffffffff720,local_8e8._0_8_,1);
    }
    if ((1 < ((uint)local_1c0 & 3) - 2) && (((ulonglong)local_1c0 & 3) != 0)) {
      uStack_560 = (ushort)(local_1c0 + -1);
      uStack_55e = (undefined6)((ulonglong)(local_1c0 + -1) >> 0x10);
      local_568._0_1_ = '\x03';
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop((longlong *)&uStack_560);
    }
    param_1[2] = CONCAT71(uStack_8f7,uStack_8f8);
    *param_1 = CONCAT71(local_908._1_7_,(byte)local_908);
    param_1[1] = CONCAT71(uStack_8ff,uStack_900);
    lVar77 = local_960;
LAB_140015a25:
    if (local_1a0 != 0) {
      __rustc::__rust_dealloc(local_198,local_1a0,1);
    }
  }
  else {
    local_908._0_1_ = (byte)pWVar42;
    local_908._1_7_ = (undefined7)((ulonglong)pWVar42 >> 8);
    stack0xfffffffffffff720 = (code *)ppWVar45;
    local_8e8._0_8_ = pWVar49;
    uStack_560 = 0xe0;
    uStack_55e = 0x14010;
    WStack_558 = (WORD)&local_908;
    uStack_556 = (ushort)((ulonglong)&local_908 >> 0x10);
    uStack_554 = (uint)((ulonglong)&local_908 >> 0x20);
    uStack_550._0_1_ = 0;
    uStack_550._1_1_ = 0xcf;
    uStack_550._2_1_ = 0x14;
    uStack_550._3_1_ = 0x40;
    uStack_550._4_4_ = 1;
    local_568._0_1_ = cVar37;
    local_568._1_1_ = uVar31;
    local_568._2_2_ = WVar32;
    local_568._4_2_ = WVar33;
    local_568._6_2_ = WVar34;
    alloc::fmt::format::format_inner
              ((ulonglong *)local_7d8,(ushort *)&DAT_14015a389,(ulonglong)&local_568);
    if ((1 < ((uint)CONCAT71(local_908._1_7_,(byte)local_908) & 3) - 2) &&
       (((byte)local_908 & 3) != 0)) {
      lVar77 = CONCAT71(local_908._1_7_,(byte)local_908) + -1;
      uStack_560 = (ushort)lVar77;
      uStack_55e = (undefined6)((ulonglong)lVar77 >> 0x10);
      local_568._0_1_ = '\x03';
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop((longlong *)&uStack_560);
    }
    param_1[2] = (ulonglong)local_7c8;
    *param_1 = (ulonglong)local_7d8;
    param_1[1] = (ulonglong)pppppppuStack_7d0;
    lVar77 = local_960;
  }
  if (local_258 != 0) {
    __rustc::__rust_dealloc(local_250,local_258,1);
  }
  if (local_148 != 0) {
    __rustc::__rust_dealloc(local_140,local_148,1);
  }
  if (local_1e0 != 0) {
    __rustc::__rust_dealloc(local_1d8,local_1e0,1);
  }
joined_r0x00014001b28d:
  if (lVar77 != 0) {
    __rustc::__rust_dealloc(local_278,lVar77,1);
  }
LAB_140015503:
  if (hMutex != (HANDLE)0x0) {
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
  }
  return param_1;
  while( true ) {
    bVar4 = (byte)*pWVar49;
    uVar40 = (uint)bVar4;
    if ((char)bVar4 < '\0') {
      uVar85 = *(byte *)((longlong)pWVar49 + 1) & 0x3f;
      if (bVar4 < 0xe0) {
        pWVar49 = pWVar49 + 1;
        uVar40 = (uVar40 & 0x1f) << 6 | uVar85;
      }
      else {
        uVar85 = (byte)pWVar49[1] & 0x3f | uVar85 << 6;
        if (bVar4 < 0xf0) {
          pWVar49 = (LPCWSTR)((longlong)pWVar49 + 3);
          uVar40 = uVar85 | (uVar40 & 0x1f) << 0xc;
        }
        else {
          pbVar46 = (byte *)((longlong)pWVar49 + 3);
          pWVar49 = pWVar49 + 2;
          uVar40 = *pbVar46 & 0x3f | uVar85 << 6 | (uVar40 & 7) << 0x12;
        }
      }
    }
    else {
      pWVar49 = (LPCWSTR)((longlong)pWVar49 + 1);
    }
    bVar36 = uVar40 - 0x30 < 10;
    if (0x40 < uVar40) {
      bVar36 = uVar40 < 0x5b;
    }
    bVar3 = uVar40 < 0x7b;
    if (uVar40 < 0x61) {
      bVar3 = bVar36;
    }
    if (bVar3) break;
LAB_14001b05d:
    local_870 = pWVar42;
    if (pWVar49 == (LPCWSTR)((longlong)pWVar42 + (longlong)local_958)) goto LAB_140015e04;
  }
  pWVar70 = pWVar42;
  if (local_958 != (undefined8 *)0x0) {
    do {
      bVar4 = (byte)*pWVar70;
      uVar40 = (uint)bVar4;
      if ((char)bVar4 < '\0') {
        uVar85 = bVar4 & 0x1f;
        uVar40 = *(byte *)((longlong)pWVar70 + 1) & 0x3f;
        if (bVar4 < 0xe0) {
          pWVar49 = pWVar70 + 1;
          uVar40 = uVar85 << 6 | uVar40;
        }
        else {
          uVar40 = (byte)pWVar70[1] & 0x3f | uVar40 << 6;
          if (bVar4 < 0xf0) {
            pWVar49 = (LPCWSTR)((longlong)pWVar70 + 3);
            uVar40 = uVar40 | uVar85 << 0xc;
          }
          else {
            pWVar49 = pWVar70 + 2;
            uVar40 = *(byte *)((longlong)pWVar70 + 3) & 0x3f | uVar40 << 6 | (bVar4 & 7) << 0x12;
          }
        }
      }
      else {
        pWVar49 = (LPCWSTR)((longlong)pWVar70 + 1);
      }
      bVar36 = uVar40 - 0x30 < 10;
      if (0x40 < uVar40) {
        bVar36 = uVar40 < 0x5b;
      }
      bVar3 = uVar40 < 0x7b;
      if (uVar40 < 0x61) {
        bVar3 = bVar36;
      }
      if ((!bVar3) &&
         ((0x3f < uVar40 - 0x20 ||
          ((0x8000000000002001U >> ((ulonglong)(uVar40 - 0x20) & 0x3f) & 1) == 0))))
      goto LAB_140015e04;
      pWVar70 = pWVar49;
    } while (pWVar49 != (LPCWSTR)((longlong)pWVar42 + (longlong)local_958));
  }
  <alloc::raw_vec::RawVecInner>::try_allocate_in
            (&local_568,(ulonglong)local_958,'\0',1,1);
  ppWVar45 = (LPCWSTR *)CONCAT62(uStack_55e,uStack_560);
  if (CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568)) == 1) {
    alloc::raw_vec::handle_error
              ((longlong)ppWVar45,CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558)));
    goto LAB_14001be41;
  }
  pWVar49 = (LPCWSTR)CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
  if (local_958 == (undefined8 *)0x0) {
    local_958 = (undefined8 *)0x0;
  }
  else {
    memcpy(pWVar49,local_870,(size_t)local_958);
  }
LAB_140015e04:
  if (local_928 != (LPCWSTR)0x0) {
    __rustc::__rust_dealloc(pvVar91,local_928,1);
  }
  ppWVar51 = local_968;
  if (ppWVar45 == (LPCWSTR *)0xffffffffffffffff) {
LAB_140015ef2:
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    pWVar49 = (LPCWSTR)__rustc::__rust_alloc(6,1);
    if (pWVar49 == (LPCWSTR)0x0) {
      alloc::raw_vec::handle_error(1,6);
      goto LAB_14001be41;
    }
    pWVar49[2] = L'ç‰¥';
    pWVar49[0] = L'æ±';
    pWVar49[1] = L'ç¥¡';
    local_238 = (LPCWSTR *)&DAT_00000006;
    puStack_228 = (undefined8 *)&DAT_00000006;
    local_230 = pWVar49;
  }
  else {
    local_568._0_1_ = (char)ppWVar45;
    local_568._1_1_ = (undefined1)((ulonglong)ppWVar45 >> 8);
    local_568._2_2_ = (WORD)((ulonglong)ppWVar45 >> 0x10);
    local_568._4_2_ = (WORD)((ulonglong)ppWVar45 >> 0x20);
    local_568._6_2_ = (WORD)((ulonglong)ppWVar45 >> 0x30);
    uStack_560 = (ushort)pWVar49;
    uStack_55e = (undefined6)((ulonglong)pWVar49 >> 0x10);
    WStack_558 = (WORD)local_958;
    uStack_556 = (ushort)((ulonglong)local_958 >> 0x10);
    uStack_554 = (uint)((ulonglong)local_958 >> 0x20);
    local_8e8._0_8_ = &local_568;
    if ((local_958 != (undefined8 *)0x0) && ((byte)*pWVar49 == 0x2d)) {
      pppppppuStack_7d0 =
           (undefined8 *******)
           <&alloc::string::String_as_core::fmt::Display>::fmt;
      local_7d8 = (undefined1  [8])local_8e8;
      std::io::stdio::_eprint((ushort *)&DAT_14015a34a,(ulonglong)local_7d8);
      lVar77 = CONCAT26(local_568._6_2_,
                        CONCAT24(local_568._4_2_,
                                 CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))
                                ));
      if (lVar77 != 0) {
        __rustc::__rust_dealloc((LPVOID)CONCAT62(uStack_55e,uStack_560),lVar77,1);
      }
      goto LAB_140015ef2;
    }
    puStack_228 = local_958;
    local_238 = ppWVar45;
    local_230 = pWVar49;
  }
  ppWVar45 = local_238;
  pvVar62 = local_818;
  <std::path::Path>::_join
            (&local_1c0,local_818,sVar92,(undefined8 *******)&DAT_14015a450,(undefined1 *)0x3);
  <std::path::Path>::_join
            (&local_220,pvVar62,sVar92,
             (undefined8 *******)&anon_261146bec65efbdf21e1c5186a43e7c0_16_llvm_12131923139401574060
             ,&DAT_00000004);
  start::_closure_4_(&local_200,ppWVar51,(size_t)ppWVar64,
                     (undefined8 *******)"armory-appearance.txt--mcc",&DAT_00000015);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar71 = (undefined8 *)__rustc::__rust_alloc(0x30,8);
  local_968 = ppWVar45;
  if (puVar71 == (undefined8 *)0x0) {
    alloc::alloc::handle_alloc_error(8,0x30);
    goto LAB_14001be41;
  }
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar50 = (undefined4 *)__rustc::__rust_alloc(5,1);
  if (puVar50 == (undefined4 *)0x0) {
    alloc::raw_vec::handle_error(1,5);
    goto LAB_14001be41;
  }
  *(undefined1 *)(puVar50 + 1) = 99;
  *puVar50 = 0x636d2d2d;
  local_908._0_1_ = (byte)local_2e0;
  local_908._1_7_ = (undefined7)((ulonglong)local_2e0 >> 8);
  uStack_900 = (undefined1)param_5;
  uStack_8ff = (undefined7)((ulonglong)param_5 >> 8);
  _local_7d8 = (_SYSTEMTIME)(ZEXT816(1) << 0x40);
  local_7c8 = (LPCWSTR *)0x0;
  WStack_558 = 0x20;
  uStack_556 = 0x6000;
  uStack_554 = 0;
  local_568._0_1_ = (char)local_7d8;
  local_568._1_1_ = (undefined1)((ulonglong)local_7d8 >> 8);
  local_568._2_2_ = (WORD)((ulonglong)local_7d8 >> 0x10);
  local_568._4_2_ = (WORD)((ulonglong)local_7d8 >> 0x20);
  local_568._6_2_ = (WORD)((ulonglong)local_7d8 >> 0x30);
  uStack_560 = 0xacd8;
  uStack_55e = 0x14015;
  cVar37 = <std::path::Display_as_core::fmt::Display>::fmt
                     ((longlong *)&local_908,&local_568);
  if (cVar37 != '\0') {
    core::result::unwrap_failed
              ("a Display implementation returned an error unexpectedly",0x37,&local_969,
               &DAT_14015a408,&PTR_s__rustc_b940084d7eb6a299eb4bfeb8e_14015ad90);
    goto LAB_14001be41;
  }
  local_8d8 = local_7c8;
  _local_8e8 = _local_7d8;
  *puVar71 = 5;
  puVar71[1] = puVar50;
  puVar71[2] = 5;
  puVar71[5] = local_7c8;
  puVar71[3] = local_7d8;
  puVar71[4] = pppppppuStack_7d0;
  local_110 = 2;
  local_100 = 2;
  local_108 = puVar71;
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<alloc::string::String,core::iter::adapters::map::Map<core::slice::iter::Iter<&str>,reach_launcher::package::s::{closure#0}>>>
  ::from_iter(&local_868,&PTR_s___armory_appearance_14015a4f0,(undefined8 *)"--profile");
  <alloc::string::String_as_core::clone::Clone>::clone
            (&local_908,(longlong)&local_200);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  ppWVar51 = (LPCWSTR *)__rustc::__rust_alloc(9,1);
  if (ppWVar51 == (LPCWSTR *)0x0) {
    alloc::raw_vec::handle_error(1,9);
    goto LAB_14001be41;
  }
  *ppWVar51 = (LPCWSTR)0x6c69666f72702d2d;
  *(undefined1 *)(ppWVar51 + 1) = 0x65;
  start::_closure_5_((undefined8 *)local_8e8,local_250,(size_t)ppWStack_248,
                     (undefined8 *******)"profile.bin--strings",(undefined1 *)0xb);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar52 = (undefined8 *)__rustc::__rust_alloc(9,1);
  if (puVar52 == (undefined8 *)0x0) {
    alloc::raw_vec::handle_error(1,9);
    goto LAB_14001be41;
  }
  *puVar52 = 0x676e697274732d2d;
  *(undefined1 *)(puVar52 + 1) = 0x73;
  start::_closure_3_(&local_568,ppWStack_218,local_210,
                     (undefined8 *******)
                     "strings_en.json--elite-looks--features-on6e--seconds86400--user-id",
                     &DAT_0000000f);
  local_7b8 = (LPCWSTR *)CONCAT71(uStack_8f7,uStack_8f8);
  local_7c8 = (LPCWSTR *)CONCAT71(local_908._1_7_,(byte)local_908);
  pcStack_7c0 = (code *)CONCAT71(uStack_8ff,uStack_900);
  local_798 = (undefined1 *)local_8e8._0_8_;
  pcStack_790 = stack0xfffffffffffff720;
  local_788 = local_8d8;
  local_768 = CONCAT26(local_568._6_2_,
                       CONCAT24(local_568._4_2_,
                                CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568)))
                      );
  pcStack_760 = (char *)CONCAT62(uStack_55e,uStack_560);
  local_758 = CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
  pcStack_7b0 = (code *)&DAT_00000009;
  ppWStack_7a0 = (LPCWSTR *)&DAT_00000009;
  ppWStack_780 = (LPCWSTR *)&DAT_00000009;
  ppWStack_770 = (LPCWSTR *)&DAT_00000009;
  _local_7d8 = (_SYSTEMTIME)(ZEXT816(5) << 0x40);
  uVar41 = local_858;
  ppWStack_7a8 = ppWVar51;
  puStack_778 = puVar52;
  if (local_868 - local_858 < 5) {
    <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
    <alloc::alloc::Global>(&local_868,local_858,5,&DAT_00000008,0x18);
    pcVar47 = local_950;
    uVar41 = local_858;
  }
  lVar61 = local_860;
  memcpy(&local_568,local_7d8,0x88);
  lVar77 = CONCAT26(local_568._6_2_,
                    CONCAT24(local_568._4_2_,
                             CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))));
  lVar87 = CONCAT62(uStack_55e,uStack_560);
  lVar53 = lVar87 - lVar77;
  if (lVar53 != 0) {
    memcpy((void *)(uVar41 * 0x18 + lVar61),&WStack_558 + lVar77 * 0xc,lVar53 * 0x18);
    uVar41 = (lVar87 + uVar41) - lVar77;
  }
  local_858 = uVar41;
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<alloc::string::String,core::iter::adapters::map::Map<core::slice::iter::Iter<&str>,reach_launcher::package::s::{closure#0}>>>
  ::from_iter((ulonglong *)local_7d8,(undefined8 *)&DAT_14015a560,(undefined8 *)"--user-name");
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(&local_868,&local_568);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  ppWVar51 = (LPCWSTR *)__rustc::__rust_alloc(0xb,1);
  if (ppWVar51 == (LPCWSTR *)0x0) {
    alloc::raw_vec::handle_error(1,0xb);
    goto LAB_14001be41;
  }
  *ppWVar51 = (LPCWSTR)0x6e2d726573752d2d;
  *(undefined4 *)((longlong)ppWVar51 + 7) = 0x656d616e;
  <alloc::string::String_as_core::clone::Clone>::clone
            (&local_568,(longlong)&local_238);
  local_788 = (LPCWSTR *)CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
  local_798 = (undefined1 *)
              CONCAT26(local_568._6_2_,
                       CONCAT24(local_568._4_2_,
                                CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568)))
                      );
  pcStack_790 = (code *)CONCAT62(uStack_55e,uStack_560);
  local_7c8 = local_2e8;
  local_7b8 = local_2e8;
  pcStack_7b0 = (code *)0xb;
  ppWStack_7a0 = (LPCWSTR *)0xb;
  _local_7d8 = (_SYSTEMTIME)(ZEXT816(3) << 0x40);
  pcStack_7c0 = pcVar47;
  ppWStack_7a8 = ppWVar51;
  if (local_868 - local_858 < 3) {
    <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
    <alloc::alloc::Global>(&local_868,local_858,3,&DAT_00000008,0x18);
  }
  uVar41 = local_858;
  pppppppuVar30 = pppppppuStack_7d0;
  local_568._0_1_ = local_7d8[0];
  local_568._1_1_ = local_7d8[1];
  local_568._2_2_ = local_7d8._2_2_;
  local_568._4_2_ = local_7d8._4_2_;
  local_568._6_2_ = local_7d8._6_2_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  local_518 = local_788;
  local_528 = local_798;
  pcStack_520 = pcStack_790;
  local_538._8_8_ = ppWStack_7a0;
  local_538._0_8_ = ppWStack_7a8;
  local_548 = SUB81(local_7b8,0);
  uStack_547 = (undefined7)((ulonglong)local_7b8 >> 8);
  pcStack_540 = pcStack_7b0;
  WStack_558 = (WORD)local_7c8;
  uStack_556 = (ushort)((ulonglong)local_7c8 >> 0x10);
  uStack_554 = (uint)((ulonglong)local_7c8 >> 0x20);
  uStack_550._0_1_ = SUB81(pcStack_7c0,0);
  uStack_550._1_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)pcStack_7c0 >> 0x20);
  lVar77 = local_7d8;
  pcVar47 = pcStack_7c0;
  if ((longlong)pppppppuStack_7d0 - lVar77 != 0) {
    memcpy((void *)(local_860 + local_858 * 0x18),&WStack_558 + lVar77 * 0xc,
           ((longlong)pppppppuStack_7d0 - lVar77) * 0x18);
    local_858 = ((longlong)pppppppuVar30 + uVar41) - lVar77;
  }
  puVar52 = puVar71 + 6;
  local_568._0_1_ = (char)puVar71;
  local_568._1_1_ = (undefined1)((ulonglong)puVar71 >> 8);
  local_568._2_2_ = (WORD)((ulonglong)puVar71 >> 0x10);
  local_568._4_2_ = (WORD)((ulonglong)puVar71 >> 0x20);
  local_568._6_2_ = (WORD)((ulonglong)puVar71 >> 0x30);
  WStack_558 = 2;
  uStack_556 = 0;
  uStack_554 = 0;
  uStack_560 = (ushort)puVar71;
  uStack_55e = (undefined6)((ulonglong)puVar71 >> 0x10);
  uStack_550._0_1_ = SUB81(puVar52,0);
  uStack_550._1_1_ = (undefined1)((ulonglong)puVar52 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)puVar52 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)puVar52 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)puVar52 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(&local_868,&local_568);
  if (*(int *)__imp__RNvNtNtNtNtCs59fwaEIuEkC_3std3sys3pal7windows7winsock8WSA_INIT != 0) {
    local_8e8[0] = 1;
    local_7d8 = (undefined1  [8])local_8e8;
    pcVar47 = (code *)&anon_0ea1136c30b0c84ba51b6a27d45f1785_0_llvm_5242054422619544231;
    <std::sys::sync::once::futex::Once>::call
              ((uint *)__imp__RNvNtNtNtNtCs59fwaEIuEkC_3std3sys3pal7windows7winsock8WSA_INIT,'\x01',
               local_7d8,0x14015c860,
               &anon_0ea1136c30b0c84ba51b6a27d45f1785_2_llvm_5242054422619544231);
  }
  s = (undefined **)&DAT_00000009;
  uVar41 = std::sys::net::connection::each_addr::
           <&str,<std::sys::net::connection::socket::UdpSocket>::bind::inner,std::sys::net::connection::socket::UdpSocket>
                     (&DAT_14015a42a,&DAT_00000009);
  ppuVar54 = s;
  if ((uVar41 & 1) == 0) {
    local_7d8 = (undefined1  [8])s;
    pSVar72 = (SOCKET *)local_7d8;
    ppuVar54 = std::sys::net::connection::each_addr::
               <&str,<std::sys::net::connection::socket::UdpSocket>::connect<&str>::_closure_0_,()>
                         (&DAT_14015a330,(char *)0xb,pSVar72,(int)pcVar47);
    uVar80 = (ushort)pSVar72;
    uVar83 = (ushort)pcVar47;
    if (ppuVar54 != (undefined **)0x0) {
      closesocket((SOCKET)s);
      goto LAB_140016685;
    }
    pSVar72 = (SOCKET *)local_7d8;
    <std::net::tcp::TcpStream>::local_addr
              ((undefined2 *)&local_568,(SOCKET *)local_7d8);
    closesocket((SOCKET)s);
    if (CONCAT11(local_568._1_1_,(char)local_568) == 2) {
      ppuVar54 = (undefined **)CONCAT62(uStack_55e,uStack_560);
      goto LAB_140016685;
    }
    iVar39 = CONCAT22(local_568._4_2_,local_568._2_2_);
    bVar36 = CONCAT11(local_568._1_1_,(char)local_568) == 1;
    if (bVar36) {
      pSVar72 = (SOCKET *)CONCAT26(WStack_558,uStack_55e);
      iVar39 = CONCAT22(local_568._6_2_,local_568._4_2_);
      uVar83 = uStack_560;
      uVar80 = uStack_556;
    }
    uVar41 = (ulonglong)uVar80 << 0x30 | (ulonglong)pSVar72 >> 0x10;
    uVar66 = (ulonglong)CONCAT24(uVar83,iVar39) | (longlong)pSVar72 << 0x30;
    bVar3 = iVar39 != 0;
    if (bVar36) {
      bVar3 = uVar66 != 0 || uVar41 != 0;
    }
    if (!bVar3) goto LAB_1400166b7;
    local_908._1_7_ = (undefined7)uVar66;
    uStack_900 = (undefined1)((ulonglong)pSVar72 >> 8);
    uStack_8ff = (undefined7)uVar41;
    uStack_8f8 = (undefined1)(uVar80 >> 8);
    _local_7d8 = (_SYSTEMTIME)(ZEXT816(1) << 0x40);
    local_7c8 = (LPCWSTR *)0x0;
    WStack_558 = 0x20;
    uStack_556 = 0x6000;
    uStack_554 = 0;
    local_568._0_1_ = (char)local_7d8;
    local_568._1_1_ = (undefined1)((ulonglong)local_7d8 >> 8);
    local_568._2_2_ = (WORD)((ulonglong)local_7d8 >> 0x10);
    local_568._4_2_ = (WORD)((ulonglong)local_7d8 >> 0x20);
    local_568._6_2_ = (WORD)((ulonglong)local_7d8 >> 0x30);
    uStack_560 = 0xacd8;
    uStack_55e = 0x14015;
    local_908._0_1_ = bVar36;
    cVar37 = <core::net::ip_addr::IpAddr_as_core::fmt::Display>::
             fmt((char *)&local_908,&local_568);
    if (cVar37 != '\0') {
      core::result::unwrap_failed
                ("a Display implementation returned an error unexpectedly",0x37,&local_969,
                 &DAT_14015a408,&PTR_s__rustc_b940084d7eb6a299eb4bfeb8e_14015ad90);
      goto LAB_14001be41;
    }
    local_8d8 = local_7c8;
    _local_8e8 = _local_7d8;
  }
  else {
LAB_140016685:
    if ((1 < ((uint)ppuVar54 & 3) - 2) && (((ulonglong)ppuVar54 & 3) != 0)) {
      uStack_560 = (ushort)(undefined1 *)((longlong)ppuVar54 + -1);
      uStack_55e = (undefined6)((ulonglong)((longlong)ppuVar54 + -1) >> 0x10);
      local_568._0_1_ = '\x03';
      <core::io::error::CustomOwner_as_core::ops::drop::Drop>::
      drop((longlong *)&uStack_560);
    }
LAB_1400166b7:
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    puVar71 = (undefined8 *)__rustc::__rust_alloc(9,1);
    if (puVar71 == (undefined8 *)0x0) {
      alloc::raw_vec::handle_error(1,9);
      goto LAB_14001be41;
    }
    *puVar71 = 0x2e302e302e373231;
    *(undefined1 *)(puVar71 + 1) = 0x31;
    stack0xfffffffffffff720 = (code *)puVar71;
    local_8e8._0_2_ = 9;
    local_8e8._2_2_ = 0;
    local_8e8._4_2_ = 0;
    local_8e8._6_2_ = 0;
    local_8d8 = (LPCWSTR *)&DAT_00000009;
  }
  local_568._0_1_ = (char)local_8e8;
  local_568._1_1_ = (undefined1)((ulonglong)local_8e8 >> 8);
  local_568._2_2_ = (WORD)((ulonglong)local_8e8 >> 0x10);
  local_568._4_2_ = (WORD)((ulonglong)local_8e8 >> 0x20);
  local_568._6_2_ = (WORD)((ulonglong)local_8e8 >> 0x30);
  uStack_560 = 0xdbb0;
  uStack_55e = 0x14001;
  WStack_558 = 0xa5cc;
  uStack_556 = 0x4015;
  uStack_554 = 1;
  uStack_550._0_1_ = 0x30;
  uStack_550._1_1_ = 0xd2;
  uStack_550._2_1_ = 0x14;
  uStack_550._3_1_ = 0x40;
  uStack_550._4_4_ = 1;
  alloc::fmt::format::format_inner
            ((ulonglong *)local_7d8,(ushort *)&DAT_14015a5ce,(ulonglong)&local_568);
  if (local_8e8._0_8_ != 0) {
    __rustc::__rust_dealloc(stack0xfffffffffffff720,local_8e8._0_8_,1);
  }
  ppWVar51 = local_7c8;
  local_870 = (LPCWSTR)local_7d8;
  local_2c0 = (undefined4 *)pppppppuStack_7d0;
  local_928 = local_250;
  local_910 = ppWStack_248;
  start::_closure_5_(&local_180,local_250,(size_t)ppWStack_248,(undefined8 *******)&DAT_14015a5db,
                     &DAT_00000009);
  sVar92 = local_210;
  ppWVar64 = ppWStack_218;
  <std::path::Path>::_join
            (&local_568,ppWStack_218,local_210,(undefined8 *******)&DAT_14015a5e4,&DAT_00000005);
  pvVar62 = (void *)CONCAT62(uStack_55e,uStack_560);
  <std::path::Path>::_join
            ((undefined8 *)local_7d8,pvVar62,CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558)),
             (undefined8 *******)&DAT_14015a5e9,&DAT_00000011);
  pppppppuVar30 = pppppppuStack_7d0;
  bVar36 = <std::path::Path>::is_file
                     ((longlong)pppppppuStack_7d0,(longlong)local_7c8);
  if (bVar36) {
    if (local_7d8 != (undefined1  [8])0x0) {
      __rustc::__rust_dealloc(pppppppuVar30,local_7d8,1);
    }
    lVar77 = CONCAT26(local_568._6_2_,
                      CONCAT24(local_568._4_2_,
                               CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))))
    ;
    if (lVar77 != 0) {
      __rustc::__rust_dealloc(pvVar62,lVar77,1);
    }
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    local_958 = (undefined8 *)__rustc::__rust_alloc(0x78,8);
    if (local_958 == (undefined8 *)0x0) {
      alloc::alloc::handle_alloc_error(8,0x78);
      goto LAB_14001be41;
    }
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    puVar52 = (undefined8 *)__rustc::__rust_alloc(0xc,1);
    puVar71 = local_958;
    if (puVar52 == (undefined8 *)0x0) {
      alloc::raw_vec::handle_error(1,0xc);
      goto LAB_14001be41;
    }
    *puVar52 = 0x2d61727478652d2d;
    *(undefined4 *)(puVar52 + 1) = 0x7370616d;
    start::_closure_3_(&local_568,ppWVar64,sVar92,(undefined8 *******)&DAT_14015a5e4,&DAT_00000005);
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    puVar55 = (undefined8 *)__rustc::__rust_alloc(0xc,1);
    if (puVar55 == (undefined8 *)0x0) {
      alloc::raw_vec::handle_error(1,0xc);
      goto LAB_14001be41;
    }
    *puVar55 = 0x2d726f6d72612d2d;
    *(undefined4 *)(puVar55 + 1) = 0x6b636170;
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    puVar56 = (undefined8 *)__rustc::__rust_alloc(0xd,1);
    if (puVar56 == (undefined8 *)0x0) {
      alloc::raw_vec::handle_error(1,0xd);
      goto LAB_14001be41;
    }
    *(undefined8 *)((longlong)puVar56 + 5) = 0x6b6361705f657469;
    *puVar56 = 0x6574696c655f7272;
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    puVar57 = (undefined8 *)__rustc::__rust_alloc(0x13,1);
    if (puVar57 == (undefined8 *)0x0) {
      alloc::raw_vec::handle_error(1,0x13);
      goto LAB_14001be41;
    }
    *puVar57 = 0x2d6574696c652d2d;
    puVar57[1] = 0x6c732d726f6d7261;
    *(undefined4 *)((longlong)puVar57 + 0xf) = 0x73746f6c;
    *puVar71 = 0xc;
    puVar71[1] = puVar52;
    puVar71[2] = 0xc;
    puVar71[3] = CONCAT26(local_568._6_2_,
                          CONCAT24(local_568._4_2_,
                                   CONCAT22(local_568._2_2_,
                                            CONCAT11(local_568._1_1_,(char)local_568))));
    puVar71[4] = CONCAT62(uStack_55e,uStack_560);
    puVar71[5] = CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
    puVar71[6] = 0xc;
    puVar71[7] = puVar55;
    puVar71[8] = 0xc;
    puVar71[9] = 0xd;
    puVar71[10] = puVar56;
    puVar71[0xb] = 0xd;
    puVar71[0xc] = 0x13;
    puVar71[0xd] = puVar57;
    puVar71[0xe] = 0x13;
    puVar76 = &DAT_00000005;
  }
  else {
    if (local_7d8 != (undefined1  [8])0x0) {
      __rustc::__rust_dealloc(pppppppuVar30,local_7d8,1);
    }
    lVar77 = CONCAT26(local_568._6_2_,
                      CONCAT24(local_568._4_2_,
                               CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))))
    ;
    if (lVar77 != 0) {
      __rustc::__rust_dealloc(pvVar62,lVar77,1);
    }
    puVar71 = (undefined8 *)&DAT_00000008;
    puVar76 = (undefined1 *)0x0;
  }
  local_f8 = puVar76;
  local_f0 = puVar71;
  local_e8 = puVar76;
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar58 = (ulonglong *)__rustc::__rust_alloc(0x48,8);
  if (puVar58 == (ulonglong *)0x0) {
    alloc::alloc::handle_alloc_error(8,0x48);
    goto LAB_14001be41;
  }
  start::_closure_5_((undefined8 *)local_7d8,local_928,(size_t)local_910,
                     (undefined8 *******)&DAT_14015a632,&DAT_00000004);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar52 = (undefined8 *)__rustc::__rust_alloc(0xe,1);
  if (puVar52 == (undefined8 *)0x0) {
    alloc::raw_vec::handle_error(1,0xe);
    goto LAB_14001be41;
  }
  local_2f0 = ppWVar51;
  *(undefined8 *)((longlong)puVar52 + 6) = 0x6b636f6c622d6863;
  *puVar52 = 0x68636e75616c2d2d;
  local_958 = puVar71;
  local_830 = puVar76;
  start::_closure_3_(&local_568,ppWVar64,sVar92,(undefined8 *******)&DAT_14015a644,&DAT_0000000c);
  puVar58[2] = (ulonglong)local_7c8;
  *puVar58 = (ulonglong)local_7d8;
  puVar58[1] = (ulonglong)pppppppuStack_7d0;
  puVar58[3] = 0xe;
  puVar58[4] = (ulonglong)puVar52;
  puVar58[5] = 0xe;
  puVar58[8] = CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
  puVar58[6] = CONCAT26(local_568._6_2_,
                        CONCAT24(local_568._4_2_,
                                 CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))
                                ));
  puVar58[7] = CONCAT62(uStack_55e,uStack_560);
  local_2d8 = 3;
  local_2c8 = 3;
  local_2d0 = puVar58;
  <alloc::vec::Vec<alloc::string::String>as_core::clone::Clone>
  ::clone((ulonglong *)local_7d8,(longlong)&local_868);
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(&local_2d8,&local_568);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  pcVar47 = (code *)__rustc::__rust_alloc(0xc,1);
  if (pcVar47 == (code *)0x0) {
    alloc::raw_vec::handle_error(1,0xc);
    goto LAB_14001be41;
  }
  *(undefined8 *)pcVar47 = 0x2d61727478652d2d;
  *(undefined4 *)(pcVar47 + 8) = 0x7370616d;
  start::_closure_3_(&local_908,ppWVar64,sVar92,(undefined8 *******)&DAT_14015a650,&DAT_00000008);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  pcVar59 = (code *)__rustc::__rust_alloc(0xc,1);
  if (pcVar59 == (code *)0x0) {
    alloc::raw_vec::handle_error(1,0xc);
    goto LAB_14001be41;
  }
  *(undefined8 *)pcVar59 = 0x732d756e656d2d2d;
  *(undefined4 *)(pcVar59 + 8) = 0x656e6563;
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar71 = (undefined8 *)__rustc::__rust_alloc(3,1);
  if (puVar71 == (undefined8 *)0x0) {
    alloc::raw_vec::handle_error(1,3);
    goto LAB_14001be41;
  }
  *(undefined1 *)((longlong)puVar71 + 2) = 0x6c;
  *(undefined2 *)puVar71 = 0x6c61;
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  pcVar60 = (char *)__rustc::__rust_alloc(0x13,1);
  if (pcVar60 == (char *)0x0) {
    alloc::raw_vec::handle_error(1,0x13);
    goto LAB_14001be41;
  }
  builtin_strncpy(pcVar60,"--armory-appearance",0x13);
  <alloc::string::String_as_core::clone::Clone>::clone
            ((size_t *)local_8e8,(longlong)&local_200);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar52 = (undefined8 *)__rustc::__rust_alloc(0x11,1);
  if (puVar52 == (undefined8 *)0x0) {
    alloc::raw_vec::handle_error(1,0x11);
    goto LAB_14001be41;
  }
  *puVar52 = 0x732d756e656d2d2d;
  puVar52[1] = 0x6c69662d656e6563;
  *(undefined1 *)(puVar52 + 2) = 0x65;
  <alloc::string::String_as_core::clone::Clone>::clone
            (&local_568,(longlong)&local_180);
  ppWStack_7a0 = (LPCWSTR *)CONCAT71(uStack_8f7,uStack_8f8);
  pcStack_7b0 = (code *)CONCAT71(local_908._1_7_,(byte)local_908);
  ppWStack_7a8 = (LPCWSTR *)CONCAT71(uStack_8ff,uStack_900);
  local_750 = (LPCWSTR)local_8e8._0_8_;
  puStack_748 = (undefined4 *)stack0xfffffffffffff720;
  local_740 = local_8d8;
  local_720 = CONCAT26(local_568._6_2_,
                       CONCAT24(local_568._4_2_,
                                CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568)))
                      );
  uStack_718 = CONCAT62(uStack_55e,uStack_560);
  local_710 = CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
  local_7c8 = (LPCWSTR *)&DAT_0000000c;
  local_7b8 = (LPCWSTR *)&DAT_0000000c;
  local_798 = &DAT_0000000c;
  local_788 = (LPCWSTR *)&DAT_0000000c;
  ppWStack_780 = (LPCWSTR *)0x3;
  ppWStack_770 = (LPCWSTR *)0x3;
  local_768 = 0x13;
  local_758 = 0x13;
  local_738 = 0x11;
  local_728 = 0x11;
  _local_7d8 = (_SYSTEMTIME)(ZEXT816(8) << 0x40);
  pcStack_7c0 = pcVar47;
  pcStack_790 = pcVar59;
  puStack_778 = puVar71;
  pcStack_760 = pcVar60;
  puStack_730 = puVar52;
  if (local_2d8 - local_2c8 < 8) {
    <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
    <alloc::alloc::Global>(&local_2d8,local_2c8,8,&DAT_00000008,0x18);
  }
  uVar41 = local_2c8;
  puVar58 = local_2d0;
  memcpy(&local_568,local_7d8,0xd0);
  lVar77 = CONCAT26(local_568._6_2_,
                    CONCAT24(local_568._4_2_,
                             CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))));
  lVar87 = CONCAT62(uStack_55e,uStack_560);
  lVar61 = lVar87 - lVar77;
  if (lVar61 != 0) {
    memcpy(puVar58 + uVar41 * 3,&WStack_558 + lVar77 * 0xc,lVar61 * 0x18);
    uVar41 = (lVar87 + uVar41) - lVar77;
  }
  local_2c8 = uVar41;
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar58 = (ulonglong *)__rustc::__rust_alloc(0x48,8);
  if (puVar58 == (ulonglong *)0x0) {
    alloc::alloc::handle_alloc_error(8,0x48);
    goto LAB_14001be41;
  }
  start::_closure_5_((undefined8 *)local_7d8,local_928,(size_t)local_910,
                     (undefined8 *******)&DAT_14015a678,&DAT_00000005);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar71 = (undefined8 *)__rustc::__rust_alloc(0xe,1);
  if (puVar71 == (undefined8 *)0x0) {
    alloc::raw_vec::handle_error(1,0xe);
    goto LAB_14001be41;
  }
  *(undefined8 *)((longlong)puVar71 + 6) = 0x6b636f6c622d6863;
  *puVar71 = 0x68636e75616c2d2d;
  start::_closure_3_(&local_568,ppWVar64,sVar92,(undefined8 *******)&DAT_14015a67d,
                     (undefined1 *)0x12);
  puVar58[2] = (ulonglong)local_7c8;
  *puVar58 = (ulonglong)local_7d8;
  puVar58[1] = (ulonglong)pppppppuStack_7d0;
  puVar58[3] = 0xe;
  puVar58[4] = (ulonglong)puVar71;
  puVar58[5] = 0xe;
  puVar58[8] = CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
  puVar58[6] = CONCAT26(local_568._6_2_,
                        CONCAT24(local_568._4_2_,
                                 CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))
                                ));
  puVar58[7] = CONCAT62(uStack_55e,uStack_560);
  local_290 = 3;
  local_280 = 3;
  local_288 = puVar58;
  <alloc::vec::Vec<alloc::string::String>as_core::clone::Clone>
  ::clone((ulonglong *)local_7d8,(longlong)&local_868);
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(&local_290,&local_568);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<alloc::string::String,core::iter::adapters::map::Map<core::slice::iter::Iter<&str>,reach_launcher::package::s::{closure#0}>>>
  ::from_iter((ulonglong *)local_7d8,&PTR_DAT_14015a6c8,(undefined8 *)"blocks");
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(&local_290,&local_568);
  <alloc::vec::Vec<alloc::string::String>as_core::clone::Clone>
  ::clone((ulonglong *)local_7d8,(longlong)&local_f8);
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(&local_290,&local_568);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar71 = (undefined8 *)__rustc::__rust_alloc(0x48,8);
  if (puVar71 == (undefined8 *)0x0) {
    alloc::alloc::handle_alloc_error(8,0x48);
    goto LAB_14001be41;
  }
  start::_closure_5_(&local_888,local_928,(size_t)local_910,(undefined8 *******)&DAT_14015a678,
                     &DAT_00000005);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar52 = (undefined8 *)__rustc::__rust_alloc(0xe,1);
  if (puVar52 == (undefined8 *)0x0) {
    alloc::raw_vec::handle_error(1,0xe);
    goto LAB_14001be41;
  }
  *(undefined8 *)((longlong)puVar52 + 6) = 0x6b636f6c622d6863;
  *puVar52 = 0x68636e75616c2d2d;
  <std::path::Path>::_join
            (&local_568,ppWVar64,sVar92,(undefined8 *******)"blocks",&DAT_00000006);
  pvVar62 = (void *)CONCAT62(uStack_55e,uStack_560);
  <std::path::Path>::_join
            ((undefined8 *)local_7d8,pvVar62,CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558)),
             (undefined8 *******)"firefight_{id}_d{difficulty}.bin",&DAT_00000020);
  pppppppuVar30 = pppppppuStack_7d0;
  local_940 = (LPCWSTR)pppppppuStack_7d0;
  local_938 = local_7c8;
  local_908._0_1_ = 0;
  local_908._1_7_ = 0;
  uStack_900 = 1;
  uStack_8ff = 0;
  uStack_8f8 = 0;
  uStack_8f7 = 0;
  local_8d8 = (LPCWSTR *)0x60000020;
  stack0xfffffffffffff720 = (code *)&PTR_<alloc::string::String>_14015acd8;
  local_8e8._0_8_ = &local_908;
  cVar37 = <std::path::Display_as_core::fmt::Display>::fmt
                     ((longlong *)&local_940,(undefined8 *)local_8e8);
  if (cVar37 != '\0') {
    core::result::unwrap_failed
              ("a Display implementation returned an error unexpectedly",0x37,&local_969,
               &DAT_14015a408,&PTR_s__rustc_b940084d7eb6a299eb4bfeb8e_14015ad90);
    goto LAB_14001be41;
  }
  local_898 = (LPCWSTR *)CONCAT71(uStack_8f7,uStack_8f8);
  local_8a8 = (LPCWSTR *)CONCAT71(local_908._1_7_,(byte)local_908);
  puStack_8a0 = (undefined8 *)CONCAT71(uStack_8ff,uStack_900);
  puVar71[2] = local_878;
  *puVar71 = local_888;
  puVar71[1] = ppWStack_880;
  puVar71[3] = 0xe;
  puVar71[4] = puVar52;
  puVar71[5] = 0xe;
  puVar71[8] = local_898;
  puVar71[6] = local_8a8;
  puVar71[7] = puStack_8a0;
  local_270 = (LPCWSTR)0x3;
  local_260 = 3;
  local_268 = puVar71;
  if (local_7d8 != (undefined1  [8])0x0) {
    __rustc::__rust_dealloc(pppppppuVar30,local_7d8,1);
  }
  lVar77 = CONCAT26(local_568._6_2_,
                    CONCAT24(local_568._4_2_,
                             CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))));
  if (lVar77 != 0) {
    __rustc::__rust_dealloc(pvVar62,lVar77,1);
  }
  <alloc::vec::Vec<alloc::string::String>as_core::clone::Clone>
  ::clone((ulonglong *)local_7d8,(longlong)&local_868);
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend((ulonglong *)&local_270,&local_568);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<alloc::string::String,core::iter::adapters::map::Map<core::slice::iter::Iter<&str>,reach_launcher::package::s::{closure#0}>>>
  ::from_iter((ulonglong *)local_7d8,&PTR_DAT_14015a760,
              (undefined8 *)
              "campaign_{id}_d{difficulty}.bin--session-players--game-players--players-table32--max-players--max-machines"
             );
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend((ulonglong *)&local_270,&local_568);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar71 = (undefined8 *)__rustc::__rust_alloc(0x48,8);
  if (puVar71 == (undefined8 *)0x0) {
    alloc::alloc::handle_alloc_error(8,0x48);
    goto LAB_14001be41;
  }
  start::_closure_5_(&local_888,local_928,(size_t)local_910,(undefined8 *******)&DAT_14015a678,
                     &DAT_00000005);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar52 = (undefined8 *)__rustc::__rust_alloc(0xe,1);
  if (puVar52 == (undefined8 *)0x0) {
    alloc::raw_vec::handle_error(1,0xe);
    goto LAB_14001be41;
  }
  *(undefined8 *)((longlong)puVar52 + 6) = 0x6b636f6c622d6863;
  *puVar52 = 0x68636e75616c2d2d;
  <std::path::Path>::_join
            (&local_568,ppWVar64,sVar92,(undefined8 *******)"blocks",&DAT_00000006);
  pvVar62 = (void *)CONCAT62(uStack_55e,uStack_560);
  <std::path::Path>::_join
            ((undefined8 *)local_7d8,pvVar62,CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558)),
             (undefined8 *******)
             "campaign_{id}_d{difficulty}.bin--session-players--game-players--players-table32--max-players--max-machines"
             ,&DAT_0000001f);
  pppppppuVar30 = pppppppuStack_7d0;
  local_940 = (LPCWSTR)pppppppuStack_7d0;
  local_938 = local_7c8;
  local_908._0_1_ = 0;
  local_908._1_7_ = 0;
  uStack_900 = 1;
  uStack_8ff = 0;
  uStack_8f8 = 0;
  uStack_8f7 = 0;
  local_8d8 = (LPCWSTR *)0x60000020;
  stack0xfffffffffffff720 = (code *)&PTR_<alloc::string::String>_14015acd8;
  cVar37 = <std::path::Display_as_core::fmt::Display>::fmt
                     ((longlong *)&local_940,(undefined8 *)local_8e8);
  if (cVar37 != '\0') {
    core::result::unwrap_failed
              ("a Display implementation returned an error unexpectedly",0x37,&local_969,
               &DAT_14015a408,&PTR_s__rustc_b940084d7eb6a299eb4bfeb8e_14015ad90);
    goto LAB_14001be41;
  }
  local_898 = (LPCWSTR *)CONCAT71(uStack_8f7,uStack_8f8);
  local_8a8 = (LPCWSTR *)CONCAT71(local_908._1_7_,(byte)local_908);
  puStack_8a0 = (undefined8 *)CONCAT71(uStack_8ff,uStack_900);
  puVar71[2] = local_878;
  *puVar71 = local_888;
  puVar71[1] = ppWStack_880;
  puVar71[3] = 0xe;
  puVar71[4] = puVar52;
  puVar71[5] = 0xe;
  puVar71[8] = local_898;
  puVar71[6] = local_8a8;
  puVar71[7] = puStack_8a0;
  local_2b8 = (LPCWSTR)0x3;
  local_2a8 = (LPCWSTR)0x3;
  local_2b0 = puVar71;
  if (local_7d8 != (undefined1  [8])0x0) {
    __rustc::__rust_dealloc(pppppppuVar30,local_7d8,1);
  }
  lVar77 = CONCAT26(local_568._6_2_,
                    CONCAT24(local_568._4_2_,
                             CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))));
  if (lVar77 != 0) {
    __rustc::__rust_dealloc(pvVar62,lVar77,1);
  }
  <alloc::vec::Vec<alloc::string::String>as_core::clone::Clone>
  ::clone((ulonglong *)local_7d8,(longlong)&local_868);
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend((ulonglong *)&local_2b8,&local_568);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar71 = (undefined8 *)__rustc::__rust_alloc(0x11,1);
  pWVar49 = local_2a8;
  if (puVar71 == (undefined8 *)0x0) {
    alloc::raw_vec::handle_error(1,0x11);
    goto LAB_14001be41;
  }
  *puVar71 = 0x612d746978652d2d;
  puVar71[1] = 0x6d61672d72657466;
  *(undefined1 *)(puVar71 + 2) = 0x65;
  if (local_2a8 == local_2b8) {
    <alloc::raw_vec::RawVec<alloc::string::String>>::grow_one
              ((ulonglong *)&local_2b8);
  }
  local_2b0[(longlong)pWVar49 * 3] = 0x11;
  local_2b0[(longlong)pWVar49 * 3 + 1] = puVar71;
  local_2b0[(longlong)pWVar49 * 3 + 2] = 0x11;
  local_2a8 = (LPCWSTR)((longlong)pWVar49 + 1);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<alloc::string::String,core::iter::adapters::map::Map<core::slice::iter::Iter<&str>,reach_launcher::package::s::{closure#0}>>>
  ::from_iter(&local_e0,(undefined8 *)&DAT_14015a800,
              (undefined8 *)
              "host--announce{master}--max-listed-players--allow-insecure-http--net-lan--port");
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar71 = (undefined8 *)__rustc::__rust_alloc(0x18,8);
  if (puVar71 == (undefined8 *)0x0) {
    alloc::alloc::handle_alloc_error(8,0x18);
    goto LAB_14001be41;
  }
  start::_closure_5_(&local_568,local_928,(size_t)local_910,
                     (undefined8 *******)
                     "host--announce{master}--max-listed-players--allow-insecure-http--net-lan--port"
                     ,&DAT_00000004);
  puVar71[2] = CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
  *puVar71 = CONCAT26(local_568._6_2_,
                      CONCAT24(local_568._4_2_,
                               CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))))
  ;
  puVar71[1] = CONCAT62(uStack_55e,uStack_560);
  local_810 = 1;
  local_800 = 1;
  local_808 = puVar71;
  <alloc::vec::Vec<alloc::string::String>as_core::clone::Clone>
  ::clone((ulonglong *)local_7d8,(longlong)&local_868);
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(&local_810,&local_568);
  <alloc::vec::Vec<alloc::string::String>as_core::clone::Clone>
  ::clone((ulonglong *)local_7d8,(longlong)&local_e0);
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(&local_810,&local_568);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<alloc::string::String,core::iter::adapters::map::Map<core::slice::iter::Iter<&str>,reach_launcher::package::s::{closure#0}>>>
  ::from_iter((ulonglong *)local_7d8,(undefined8 *)&DAT_14015a8d0,
              (undefined8 *)"--upnp--net-lan-addr");
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(&local_810,&local_568);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<alloc::string::String,core::iter::adapters::map::Map<core::slice::iter::Iter<&str>,reach_launcher::package::s::{closure#0}>>>
  ::from_iter((ulonglong *)local_7d8,&PTR_s___upnp__net_lan_addr_14015a958,
              (undefined8 *)"--server-name{name}");
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  puVar58 = &local_810;
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(puVar58,&local_568);
  puVar71 = &local_568;
  pvVar62 = (void *)<u16>::_fmt((uint)CONCAT62((int6)((ulonglong)puVar58 >> 0x10),0x9c4b),
                                (longlong)puVar71,5);
  if ((longlong)puVar71 < 0) {
    lVar77 = 0;
LAB_14001796d:
    alloc::raw_vec::handle_error(lVar77,puVar71);
    goto LAB_14001be41;
  }
  if (puVar71 == (undefined8 *)0x0) {
    pvVar63 = (void *)0x1;
  }
  else {
    __rustc::__rust_no_alloc_shim_is_unstable_v2();
    lVar77 = 1;
    pvVar63 = (void *)__rustc::__rust_alloc((longlong)puVar71,1);
    if (pvVar63 == (void *)0x0) goto LAB_14001796d;
    memcpy(pvVar63,pvVar62,(size_t)puVar71);
  }
  uVar41 = local_800;
  if (local_800 == local_810) {
    <alloc::raw_vec::RawVec<alloc::string::String>>::grow_one
              (&local_810);
  }
  local_808[uVar41 * 3] = puVar71;
  local_808[uVar41 * 3 + 1] = pvVar63;
  local_808[uVar41 * 3 + 2] = puVar71;
  local_800 = uVar41 + 1;
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<alloc::string::String,core::iter::adapters::map::Map<core::slice::iter::Iter<&str>,reach_launcher::package::s::{closure#0}>>>
  ::from_iter((ulonglong *)local_7d8,&PTR_s___server_name_name__14015a990,
              (undefined8 *)"guest--join{id}");
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(&local_810,&local_568);
  <alloc::vec::Vec<alloc::string::String>as_core::clone::Clone>
  ::clone((ulonglong *)local_7d8,(longlong)&local_f8);
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(&local_810,&local_568);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar71 = (undefined8 *)__rustc::__rust_alloc(0x18,8);
  if (puVar71 == (undefined8 *)0x0) {
    alloc::alloc::handle_alloc_error(8,0x18);
    goto LAB_14001be41;
  }
  start::_closure_5_(&local_568,local_928,(size_t)local_910,(undefined8 *******)"guest--join{id}",
                     &DAT_00000005);
  puVar71[2] = CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
  *puVar71 = CONCAT26(local_568._6_2_,
                      CONCAT24(local_568._4_2_,
                               CONCAT22(local_568._2_2_,CONCAT11(local_568._1_1_,(char)local_568))))
  ;
  puVar71[1] = CONCAT62(uStack_55e,uStack_560);
  local_848 = 1;
  local_838 = 1;
  lVar77 = local_860 + local_858 * 0x18;
  local_568._0_1_ = (char)local_860;
  local_568._1_1_ = (undefined1)((ulonglong)local_860 >> 8);
  local_568._2_2_ = (WORD)((ulonglong)local_860 >> 0x10);
  local_568._4_2_ = (WORD)((ulonglong)local_860 >> 0x20);
  local_568._6_2_ = (WORD)((ulonglong)local_860 >> 0x30);
  WStack_558 = (WORD)local_868;
  uStack_556 = (ushort)(local_868 >> 0x10);
  uStack_554 = (uint)(local_868 >> 0x20);
  uStack_560 = (ushort)local_860;
  uStack_55e = (undefined6)((ulonglong)local_860 >> 0x10);
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  local_840 = puVar71;
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(&local_848,&local_568);
  lVar77 = local_d8 + local_d0 * 0x18;
  local_568._0_1_ = (char)local_d8;
  local_568._1_1_ = (undefined1)((ulonglong)local_d8 >> 8);
  local_568._2_2_ = (WORD)((ulonglong)local_d8 >> 0x10);
  local_568._4_2_ = (WORD)((ulonglong)local_d8 >> 0x20);
  local_568._6_2_ = (WORD)((ulonglong)local_d8 >> 0x30);
  WStack_558 = (WORD)local_e0;
  uStack_556 = (ushort)(local_e0 >> 0x10);
  uStack_554 = (uint)(local_e0 >> 0x20);
  uStack_560 = (ushort)local_d8;
  uStack_55e = (undefined6)((ulonglong)local_d8 >> 0x10);
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(&local_848,&local_568);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<alloc::string::String,core::iter::adapters::map::Map<core::slice::iter::Iter<&str>,reach_launcher::package::s::{closure#0}>>>
  ::from_iter((ulonglong *)local_7d8,(undefined8 *)&DAT_14015aa20,(undefined8 *)&DAT_14015aa80);
  lVar77 = (longlong)pppppppuStack_7d0 + (longlong)local_7c8 * 0x18;
  local_568._0_1_ = local_7d8[8];
  local_568._1_1_ = local_7d8[9];
  local_568._2_2_ = local_7d8._10_2_;
  local_568._4_2_ = local_7d8._12_2_;
  local_568._6_2_ = local_7d8._14_2_;
  WStack_558 = local_7d8._0_2_;
  uStack_556 = local_7d8._2_2_;
  uStack_554 = local_7d8._4_4_;
  uStack_560 = local_7d8._8_2_;
  uStack_55e = local_7d8._10_6_;
  uStack_550._0_1_ = (undefined1)lVar77;
  uStack_550._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
  uStack_550._2_1_ = (undefined1)((ulonglong)lVar77 >> 0x10);
  uStack_550._3_1_ = (undefined1)((ulonglong)lVar77 >> 0x18);
  uStack_550._4_4_ = (undefined4)((ulonglong)lVar77 >> 0x20);
  <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
  ::spec_extend(&local_848,&local_568);
  __rustc::__rust_no_alloc_shim_is_unstable_v2();
  puVar71 = (undefined8 *)&DAT_0000000a;
  lVar77 = 1;
  puVar52 = (undefined8 *)__rustc::__rust_alloc(10,1);
  uVar41 = local_838;
  if (puVar52 != (undefined8 *)0x0) {
    *puVar52 = 0x6c65722d6f6e2d2d;
    *(undefined2 *)(puVar52 + 1) = 0x7961;
    if (local_838 == local_848) {
      <alloc::raw_vec::RawVec<alloc::string::String>>::grow_one
                (&local_848);
    }
    local_840[uVar41 * 3] = 10;
    local_840[uVar41 * 3 + 1] = puVar52;
    local_840[uVar41 * 3 + 2] = 10;
    local_838 = uVar41 + 1;
    puVar71 = &local_568;
    pvVar62 = (void *)<u16>::_fmt((uint)CONCAT62((int6)(uVar41 * 3 >> 0x10),0x9c4c),
                                  (longlong)puVar71,5);
    if ((longlong)puVar71 < 0) {
      lVar77 = 0;
    }
    else {
      if (puVar71 == (undefined8 *)0x0) {
        pvVar63 = (void *)0x1;
      }
      else {
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        lVar77 = 1;
        pvVar63 = (void *)__rustc::__rust_alloc((longlong)puVar71,1);
        if (pvVar63 == (void *)0x0) goto LAB_140017d42;
        memcpy(pvVar63,pvVar62,(size_t)puVar71);
      }
      uVar41 = local_838;
      if (local_838 == local_848) {
        <alloc::raw_vec::RawVec<alloc::string::String>>::
        grow_one(&local_848);
      }
      local_840[uVar41 * 3] = puVar71;
      local_840[uVar41 * 3 + 1] = pvVar63;
      local_840[uVar41 * 3 + 2] = puVar71;
      local_838 = uVar41 + 1;
      __rustc::__rust_no_alloc_shim_is_unstable_v2();
      puVar71 = (undefined8 *)0xb;
      lVar77 = 1;
      puVar55 = (undefined8 *)__rustc::__rust_alloc(0xb,1);
      uVar41 = local_838;
      puVar52 = local_958;
      if (puVar55 != (undefined8 *)0x0) {
        *puVar55 = 0x672d7478656e2d2d;
        *(undefined4 *)((longlong)puVar55 + 7) = 0x656d6167;
        if (local_838 == local_848) {
          <alloc::raw_vec::RawVec<alloc::string::String>>::
          grow_one(&local_848);
        }
        local_840[uVar41 * 3] = 0xb;
        local_840[uVar41 * 3 + 1] = puVar55;
        local_840[uVar41 * 3 + 2] = 0xb;
        local_838 = uVar41 + 1;
        puVar71 = puVar52 + (longlong)local_830 * 3;
        local_568._0_1_ = (char)puVar52;
        local_568._1_1_ = (undefined1)((ulonglong)puVar52 >> 8);
        local_568._2_2_ = (WORD)((ulonglong)puVar52 >> 0x10);
        local_568._4_2_ = (WORD)((ulonglong)puVar52 >> 0x20);
        local_568._6_2_ = (WORD)((ulonglong)puVar52 >> 0x30);
        WStack_558 = (WORD)local_830;
        uStack_556 = (ushort)((ulonglong)local_830 >> 0x10);
        uStack_554 = (uint)((ulonglong)local_830 >> 0x20);
        uStack_560 = (ushort)puVar52;
        uStack_55e = (undefined6)((ulonglong)puVar52 >> 0x10);
        uStack_550._0_1_ = SUB81(puVar71,0);
        uStack_550._1_1_ = (undefined1)((ulonglong)puVar71 >> 8);
        uStack_550._2_1_ = (undefined1)((ulonglong)puVar71 >> 0x10);
        uStack_550._3_1_ = (undefined1)((ulonglong)puVar71 >> 0x18);
        uStack_550._4_4_ = (undefined4)((ulonglong)puVar71 >> 0x20);
        <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_extend::SpecExtend<alloc::string::String,alloc::vec::into_iter::IntoIter<alloc::string::String>>>
        ::spec_extend(&local_848,&local_568);
        local_568._0_1_ = '\0';
        local_568._1_1_ = 0;
        local_568._2_2_ = 0;
        local_568._4_2_ = 0;
        local_568._6_2_ = 0;
        uStack_560 = 6;
        uStack_55e = 0;
        WStack_558 = 0xa632;
        uStack_556 = 0x4015;
        uStack_554 = 1;
        uStack_550._0_1_ = 4;
        uStack_550._1_1_ = 0;
        uStack_550._2_1_ = 0;
        uStack_550._3_1_ = 0;
        uStack_550._4_4_ = 0;
        plVar88 = (longlong *)&local_548;
        local_548 = SUB81(&local_2d8,0);
        uStack_547 = (undefined7)((ulonglong)&local_2d8 >> 8);
        pcStack_540 = (code *)&DAT_14015a678;
        local_538._8_8_ = &local_290;
        local_538._0_8_ = 5;
        local_528 = &DAT_14015aa95;
        pcStack_520 = (code *)&DAT_00000009;
        local_518 = &local_270;
        ppWStack_510 = (LPCWSTR *)&DAT_14015aa9e;
        local_508 = (undefined8 *)&DAT_00000008;
        ppWStack_500 = &local_2b8;
        local_4f8 = "host--announce{master}--max-listed-players--allow-insecure-http--net-lan--port"
        ;
        local_4f0 = 4;
        local_4e8 = &local_810;
        local_4e0 = &DAT_14015aaa6;
        local_4d8 = 4;
        local_4d0 = &local_848;
        lVar77 = 1;
        do {
          if ((undefined1 *)plVar88[-2] == (undefined1 *)0x0) break;
          lVar87 = *plVar88;
          pcStack_7f0 = (code *)plVar88[-1];
          pppppppuStack_7d0 = (undefined8 *******)<&str_as_core::fmt::Display>::fmt
          ;
          local_7d8 = (undefined1  [8])&local_7f8;
          local_7f8 = (undefined1 *)plVar88[-2];
          alloc::fmt::format::format_inner
                    (&local_b0,(ushort *)&DAT_14015ac9d,(ulonglong)local_7d8);
          pppppppuVar30 = local_a8;
          <std::path::Path>::_join
                    ((undefined8 *)local_8e8,local_928,(size_t)local_910,local_a8,local_a0);
          if (local_b0 != 0) {
            __rustc::__rust_dealloc(pppppppuVar30,local_b0,1);
          }
          ppWVar51 = local_8d8;
          pvVar91 = stack0xfffffffffffff720;
          alloc::str::join_generic_copy::
          <str,u8,alloc::string::String>
                    ((ulonglong *)local_7d8,*(longlong *)(lVar87 + 8),*(ulonglong *)(lVar87 + 0x10),
                     (undefined4 *)&DAT_14015a428,2);
          auVar78 = local_7d8;
          if ((ulonglong)((longlong)local_7d8 - (longlong)local_7c8) < 2) {
            <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
            <alloc::alloc::Global>
                      ((ulonglong *)local_7d8,(ulonglong)local_7c8,2,(LPVOID)0x1,1);
            auVar78 = local_7d8;
          }
          pppppppuVar30 = pppppppuStack_7d0;
          *(undefined2 *)((longlong)pppppppuStack_7d0 + (longlong)local_7c8) = 0xa0d;
          puVar44 = std::fs::write::inner
                              ((longlong)pvVar91,(undefined *)ppWVar51,
                               (undefined *)pppppppuStack_7d0,(undefined *)((longlong)local_7c8 + 2)
                              );
          if (auVar78 != (undefined1  [8])0x0) {
            __rustc::__rust_dealloc(pppppppuVar30,auVar78,1);
          }
          if (puVar44 != (undefined *)0x0) {
            local_568._0_1_ = (char)lVar77;
            local_568._1_1_ = (undefined1)((ulonglong)lVar77 >> 8);
            local_568._2_2_ = (WORD)((ulonglong)lVar77 >> 0x10);
            local_568._4_2_ = (WORD)((ulonglong)lVar77 >> 0x20);
            local_568._6_2_ = (WORD)((ulonglong)lVar77 >> 0x30);
            local_7d8 = (undefined1  [8])&local_2a0;
            pppppppuStack_7d0 =
                 (undefined8 *******)
                 <core::io::error::Error_as_core::fmt::Display>::
                 fmt;
            local_2a0 = puVar44;
            alloc::fmt::format::format_inner
                      (&local_908,(ushort *)&DAT_14015a3b2,(ulonglong)local_7d8);
            pWVar49 = local_870;
            ppWVar51 = local_968;
            local_8a8 = (LPCWSTR *)CONCAT71(local_908._1_7_,(byte)local_908);
            puStack_8a0 = (undefined8 *)CONCAT71(uStack_8ff,uStack_900);
            local_898 = (LPCWSTR *)CONCAT71(uStack_8f7,uStack_8f8);
            local_940 = local_928;
            local_938 = local_910;
            pppppppuStack_7d0 =
                 (undefined8 *******)
                 &<alloc::string::String_as_core::fmt::Display>::
                  fmt;
            local_7d8 = (undefined1  [8])&local_8a8;
            local_7c8 = &local_940;
            pcStack_7c0 = <std::path::Display_as_core::fmt::Display>
                          ::fmt;
            alloc::fmt::format::format_inner
                      ((ulonglong *)&local_888,(ushort *)&DAT_14015a3d7,(ulonglong)local_7d8);
            hMutex = local_828;
            param_1 = local_850;
            if (local_8a8 != (LPCWSTR *)0x0) {
              __rustc::__rust_dealloc(puStack_8a0,local_8a8,1);
            }
            if ((1 < ((uint)local_2a0 & 3) - 2) && (((ulonglong)local_2a0 & 3) != 0)) {
              pppppppuStack_7d0 = (undefined8 *******)(local_2a0 + -1);
              local_7d8[0] = 3;
              <core::io::error::CustomOwner_as_core::ops::drop::Drop>
              ::drop((longlong *)(local_7d8 + 8));
            }
            param_1[2] = (ulonglong)local_878;
            *param_1 = (ulonglong)local_888;
            param_1[1] = (ulonglong)ppWStack_880;
            if (local_8e8._0_8_ != 0) {
              __rustc::__rust_dealloc(pvVar91,local_8e8._0_8_,1);
            }
            bVar36 = true;
            lVar77 = local_960;
            goto LAB_14001ab56;
          }
          if (local_8e8._0_8_ != 0) {
            __rustc::__rust_dealloc(pvVar91,local_8e8._0_8_,1);
          }
          plVar88 = plVar88 + 3;
          lVar77 = lVar77 + 1;
        } while (lVar77 != 7);
        <std::path::Path>::_join
                  (&local_568,local_818,local_820,
                   (undefined8 *******)
                   &anon_261146bec65efbdf21e1c5186a43e7c0_17_llvm_12131923139401574060,&DAT_00000006
                  );
        pvVar62 = (void *)CONCAT62(uStack_55e,uStack_560);
        <std::path::Path>::_join
                  ((undefined8 *)local_7d8,pvVar62,
                   CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558)),
                   (undefined8 *******)&DAT_14015aaaa,&DAT_00000007);
        pppppppuVar30 = pppppppuStack_7d0;
        <std::path::Path>::_join
                  ((undefined8 *)local_8e8,pppppppuStack_7d0,(size_t)local_7c8,
                   (undefined8 *******)&DAT_14015aab1,&DAT_0000000a);
        pcVar47 = stack0xfffffffffffff720;
        local_888 = stack0xfffffffffffff720;
        ppWStack_880 = local_8d8;
        local_8a8 = (LPCWSTR *)0x0;
        puStack_8a0 = (undefined8 *)0x1;
        local_898 = (LPCWSTR *)0x0;
        uStack_8f8 = 0x20;
        uStack_8f7 = 0x600000;
        local_908._0_1_ = (byte)&local_8a8;
        local_908._1_7_ = (undefined7)((ulonglong)&local_8a8 >> 8);
        uStack_900 = 0xd8;
        uStack_8ff = 0x14015ac;
        cVar37 = <std::path::Display_as_core::fmt::Display>::fmt
                           ((longlong *)&local_888,&local_908);
        ppWVar64 = local_898;
        puVar71 = puStack_8a0;
        ppWVar51 = local_8a8;
        if (cVar37 != '\0') {
          core::result::unwrap_failed
                    ("a Display implementation returned an error unexpectedly",0x37,&local_969,
                     &DAT_14015a408,&PTR_s__rustc_b940084d7eb6a299eb4bfeb8e_14015ad90);
          goto LAB_14001be41;
        }
        if ((longlong)local_898 < 0) {
          lVar77 = 0;
LAB_1400182f6:
          alloc::raw_vec::handle_error(lVar77,ppWVar64);
          goto LAB_14001be41;
        }
        if (local_898 == (LPCWSTR *)0x0) {
          puVar52 = (undefined8 *)0x1;
        }
        else {
          __rustc::__rust_no_alloc_shim_is_unstable_v2();
          lVar77 = 1;
          puVar52 = (undefined8 *)__rustc::__rust_alloc((longlong)ppWVar64,1);
          if (puVar52 == (undefined8 *)0x0) goto LAB_1400182f6;
          if ((ulonglong)((longlong)puVar71 - (longlong)puVar52) < 0xffffffffffffffe1 &&
              &DAT_00000007 < ppWVar64) {
            if (&DAT_0000001f < ppWVar64) {
              ppWVar86 = (LPCWSTR *)((ulonglong)ppWVar64 & 0x7fffffffffffffe0);
              ppWVar45 = (LPCWSTR *)0x0;
              do {
                pbVar46 = (byte *)((longlong)puVar71 + (longlong)ppWVar45);
                bVar4 = pbVar46[1];
                bVar5 = pbVar46[2];
                bVar6 = pbVar46[3];
                bVar7 = pbVar46[4];
                bVar8 = pbVar46[5];
                bVar9 = pbVar46[6];
                bVar10 = pbVar46[7];
                bVar95 = pbVar46[8];
                bVar96 = pbVar46[9];
                bVar97 = pbVar46[10];
                bVar98 = pbVar46[0xb];
                bVar99 = pbVar46[0xc];
                bVar100 = pbVar46[0xd];
                bVar101 = pbVar46[0xe];
                bVar102 = pbVar46[0xf];
                pbVar82 = (byte *)((longlong)(puVar71 + 2) + (longlong)ppWVar45);
                bVar11 = *pbVar82;
                bVar12 = pbVar82[1];
                bVar13 = pbVar82[2];
                bVar14 = pbVar82[3];
                bVar15 = pbVar82[4];
                bVar16 = pbVar82[5];
                bVar17 = pbVar82[6];
                bVar18 = pbVar82[7];
                bVar19 = pbVar82[8];
                bVar20 = pbVar82[9];
                bVar21 = pbVar82[10];
                bVar22 = pbVar82[0xb];
                bVar23 = pbVar82[0xc];
                bVar24 = pbVar82[0xd];
                bVar25 = pbVar82[0xe];
                bVar26 = pbVar82[0xf];
                bVar103 = -(*pbVar46 == '\\');
                bVar104 = -(bVar4 == '\\');
                bVar105 = -(bVar5 == '\\');
                bVar106 = -(bVar6 == '\\');
                bVar107 = -(bVar7 == '\\');
                bVar108 = -(bVar8 == '\\');
                bVar109 = -(bVar9 == '\\');
                bVar110 = -(bVar10 == '\\');
                bVar111 = -(bVar95 == '\\');
                bVar112 = -(bVar96 == '\\');
                bVar113 = -(bVar97 == '\\');
                bVar114 = -(bVar98 == '\\');
                bVar115 = -(bVar99 == '\\');
                bVar116 = -(bVar100 == '\\');
                bVar117 = -(bVar101 == '\\');
                bVar118 = -(bVar102 == '\\');
                bVar119 = -(bVar11 == '\\');
                bVar120 = -(bVar12 == '\\');
                bVar121 = -(bVar13 == '\\');
                bVar122 = -(bVar14 == '\\');
                bVar123 = -(bVar15 == '\\');
                bVar124 = -(bVar16 == '\\');
                bVar125 = -(bVar17 == '\\');
                bVar126 = -(bVar18 == '\\');
                bVar127 = -(bVar19 == '\\');
                bVar128 = -(bVar20 == '\\');
                bVar129 = -(bVar21 == '\\');
                bVar130 = -(bVar22 == '\\');
                bVar131 = -(bVar23 == '\\');
                bVar132 = -(bVar24 == '\\');
                bVar133 = -(bVar25 == '\\');
                bVar134 = -(bVar26 == '\\');
                pbVar82 = (byte *)((longlong)puVar52 + (longlong)ppWVar45);
                *pbVar82 = bVar103 & '/' | ~bVar103 & *pbVar46;
                pbVar82[1] = bVar104 & '/' | ~bVar104 & bVar4;
                pbVar82[2] = bVar105 & '/' | ~bVar105 & bVar5;
                pbVar82[3] = bVar106 & '/' | ~bVar106 & bVar6;
                pbVar82[4] = bVar107 & '/' | ~bVar107 & bVar7;
                pbVar82[5] = bVar108 & '/' | ~bVar108 & bVar8;
                pbVar82[6] = bVar109 & '/' | ~bVar109 & bVar9;
                pbVar82[7] = bVar110 & '/' | ~bVar110 & bVar10;
                pbVar82[8] = bVar111 & '/' | ~bVar111 & bVar95;
                pbVar82[9] = bVar112 & '/' | ~bVar112 & bVar96;
                pbVar82[10] = bVar113 & '/' | ~bVar113 & bVar97;
                pbVar82[0xb] = bVar114 & '/' | ~bVar114 & bVar98;
                pbVar82[0xc] = bVar115 & '/' | ~bVar115 & bVar99;
                pbVar82[0xd] = bVar116 & '/' | ~bVar116 & bVar100;
                pbVar82[0xe] = bVar117 & '/' | ~bVar117 & bVar101;
                pbVar82[0xf] = bVar118 & '/' | ~bVar118 & bVar102;
                pbVar46 = (byte *)((longlong)(puVar52 + 2) + (longlong)ppWVar45);
                *pbVar46 = bVar119 & '/' | ~bVar119 & bVar11;
                pbVar46[1] = bVar120 & '/' | ~bVar120 & bVar12;
                pbVar46[2] = bVar121 & '/' | ~bVar121 & bVar13;
                pbVar46[3] = bVar122 & '/' | ~bVar122 & bVar14;
                pbVar46[4] = bVar123 & '/' | ~bVar123 & bVar15;
                pbVar46[5] = bVar124 & '/' | ~bVar124 & bVar16;
                pbVar46[6] = bVar125 & '/' | ~bVar125 & bVar17;
                pbVar46[7] = bVar126 & '/' | ~bVar126 & bVar18;
                pbVar46[8] = bVar127 & '/' | ~bVar127 & bVar19;
                pbVar46[9] = bVar128 & '/' | ~bVar128 & bVar20;
                pbVar46[10] = bVar129 & '/' | ~bVar129 & bVar21;
                pbVar46[0xb] = bVar130 & '/' | ~bVar130 & bVar22;
                pbVar46[0xc] = bVar131 & '/' | ~bVar131 & bVar23;
                pbVar46[0xd] = bVar132 & '/' | ~bVar132 & bVar24;
                pbVar46[0xe] = bVar133 & '/' | ~bVar133 & bVar25;
                pbVar46[0xf] = bVar134 & '/' | ~bVar134 & bVar26;
                ppWVar45 = ppWVar45 + 4;
              } while (ppWVar86 != ppWVar45);
              if (ppWVar64 == ppWVar86) goto LAB_140018786;
              ppWVar45 = ppWVar86;
              if (((ulonglong)ppWVar64 & 0x18) == 0) goto LAB_1400186fe;
            }
            else {
              ppWVar45 = (LPCWSTR *)0x0;
            }
            ppWVar86 = (LPCWSTR *)((ulonglong)ppWVar64 & 0x7ffffffffffffff8);
            do {
              pbVar46 = (byte *)((longlong)puVar71 + (longlong)ppWVar45);
              bVar4 = pbVar46[1];
              bVar5 = pbVar46[2];
              bVar6 = pbVar46[3];
              bVar7 = pbVar46[4];
              bVar8 = pbVar46[5];
              bVar9 = pbVar46[6];
              bVar10 = pbVar46[7];
              bVar95 = -(*pbVar46 == '\\');
              bVar96 = -(bVar4 == '\\');
              bVar97 = -(bVar5 == '\\');
              bVar98 = -(bVar6 == '\\');
              bVar99 = -(bVar7 == '\\');
              bVar100 = -(bVar8 == '\\');
              bVar101 = -(bVar9 == '\\');
              bVar102 = -(bVar10 == '\\');
              pbVar82 = (byte *)((longlong)puVar52 + (longlong)ppWVar45);
              *pbVar82 = bVar95 & '/' | ~bVar95 & *pbVar46;
              pbVar82[1] = bVar96 & '/' | ~bVar96 & bVar4;
              pbVar82[2] = bVar97 & '/' | ~bVar97 & bVar5;
              pbVar82[3] = bVar98 & '/' | ~bVar98 & bVar6;
              pbVar82[4] = bVar99 & '/' | ~bVar99 & bVar7;
              pbVar82[5] = bVar100 & '/' | ~bVar100 & bVar8;
              pbVar82[6] = bVar101 & '/' | ~bVar101 & bVar9;
              pbVar82[7] = bVar102 & '/' | ~bVar102 & bVar10;
              ppWVar45 = ppWVar45 + 1;
            } while (ppWVar86 != ppWVar45);
            if (ppWVar64 == ppWVar86) goto LAB_140018786;
          }
          else {
            ppWVar86 = (LPCWSTR *)0x0;
          }
LAB_1400186fe:
          uVar41 = (ulonglong)ppWVar64 & 3;
          ppWVar45 = ppWVar86;
          if (uVar41 != 0) {
            do {
              cVar37 = *(char *)((longlong)puVar71 + (longlong)ppWVar45);
              if (*(char *)((longlong)puVar71 + (longlong)ppWVar45) == '\\') {
                cVar37 = '/';
              }
              *(char *)((longlong)puVar52 + (longlong)ppWVar45) = cVar37;
              ppWVar45 = (LPCWSTR *)((longlong)ppWVar45 + 1);
              uVar41 = uVar41 - 1;
            } while (uVar41 != 0);
          }
          if ((ulonglong)((longlong)ppWVar86 - (longlong)ppWVar64) < 0xfffffffffffffffd) {
            do {
              cVar37 = *(char *)((longlong)puVar71 + (longlong)ppWVar45);
              if (*(char *)((longlong)puVar71 + (longlong)ppWVar45) == '\\') {
                cVar37 = '/';
              }
              *(char *)((longlong)puVar52 + (longlong)ppWVar45) = cVar37;
              cVar37 = ((undefined1 *)((longlong)puVar71 + 1))[(longlong)ppWVar45];
              if (cVar37 == '\\') {
                cVar37 = '/';
              }
              ((undefined1 *)((longlong)puVar52 + 1))[(longlong)ppWVar45] = cVar37;
              cVar37 = *(char *)((longlong)((longlong)puVar71 + 2) + (longlong)ppWVar45);
              if (cVar37 == '\\') {
                cVar37 = '/';
              }
              ((undefined1 *)((longlong)puVar52 + 2))[(longlong)ppWVar45] = cVar37;
              cVar37 = ((undefined1 *)((longlong)puVar71 + 3))[(longlong)ppWVar45];
              if (cVar37 == '\\') {
                cVar37 = '/';
              }
              ((undefined1 *)((longlong)puVar52 + 3))[(longlong)ppWVar45] = cVar37;
              ppWVar45 = (LPCWSTR *)((longlong)ppWVar45 + 4);
            } while (ppWVar64 != ppWVar45);
          }
        }
LAB_140018786:
        local_958 = puVar52;
        if (ppWVar51 != (LPCWSTR *)0x0) {
          __rustc::__rust_dealloc(puVar71,ppWVar51,1);
        }
        if (local_8e8._0_8_ != 0) {
          __rustc::__rust_dealloc(pcVar47,local_8e8._0_8_,1);
        }
        if (local_7d8 != (undefined1  [8])0x0) {
          __rustc::__rust_dealloc(pppppppuVar30,local_7d8,1);
        }
        lVar77 = CONCAT26(local_568._6_2_,
                          CONCAT24(local_568._4_2_,
                                   CONCAT22(local_568._2_2_,
                                            CONCAT11(local_568._1_1_,(char)local_568))));
        if (lVar77 != 0) {
          __rustc::__rust_dealloc(pvVar62,lVar77,1);
        }
        local_568._0_1_ = (char)local_958;
        local_568._1_1_ = (undefined1)((ulonglong)local_958 >> 8);
        local_568._2_2_ = (WORD)((ulonglong)local_958 >> 0x10);
        local_568._4_2_ = (WORD)((ulonglong)local_958 >> 0x20);
        local_568._6_2_ = (WORD)((ulonglong)local_958 >> 0x30);
        uStack_560 = (ushort)(undefined1 *)((longlong)local_958 + (longlong)ppWVar64);
        uStack_55e = (undefined6)((ulonglong)((longlong)local_958 + (longlong)ppWVar64) >> 0x10);
        WStack_558 = 0xaabb;
        uStack_556 = 0x4015;
        uStack_554 = 1;
        uStack_550._0_1_ = 2;
        uStack_550._1_1_ = 0;
        uStack_550._2_1_ = 0;
        uStack_550._3_1_ = 0;
        uStack_550._4_4_ = 0;
        <alloc::string::String_as_core::iter::traits::collect::FromIterator<alloc::string::String>>
        ::from_iter::
        <core::iter::adapters::map::Map<core::str::iter::Bytes,reach_launcher::package::percent::_closure_0_>>
                  ((ulonglong *)local_8e8,&local_568);
        local_568._0_1_ = (char)local_230;
        local_568._1_1_ = (undefined1)((ulonglong)local_230 >> 8);
        local_568._2_2_ = (WORD)((ulonglong)local_230 >> 0x10);
        local_568._4_2_ = (WORD)((ulonglong)local_230 >> 0x20);
        local_568._6_2_ = (WORD)((ulonglong)local_230 >> 0x30);
        uStack_560 = (ushort)(byte *)((longlong)puStack_228 + (longlong)local_230);
        uStack_55e = (undefined6)((ulonglong)((longlong)puStack_228 + (longlong)local_230) >> 0x10);
        WStack_558 = 1;
        uStack_556 = 0;
        uStack_554 = 0;
        uStack_550._0_1_ = 0;
        uStack_550._1_1_ = 0;
        uStack_550._2_1_ = 0;
        uStack_550._3_1_ = 0;
        uStack_550._4_4_ = 0;
        local_948 = ppWVar64;
        <alloc::string::String_as_core::iter::traits::collect::FromIterator<alloc::string::String>>
        ::from_iter::
        <core::iter::adapters::map::Map<core::str::iter::Bytes,reach_launcher::package::percent::_closure_0_>>
                  ((ulonglong *)local_7d8,&local_568);
        local_568._0_1_ = (char)local_8e8;
        local_568._1_1_ = (undefined1)((ulonglong)local_8e8 >> 8);
        local_568._2_2_ = (WORD)((ulonglong)local_8e8 >> 0x10);
        local_568._4_2_ = (WORD)((ulonglong)local_8e8 >> 0x20);
        local_568._6_2_ = (WORD)((ulonglong)local_8e8 >> 0x30);
        uStack_560 = 0xdbb0;
        uStack_55e = 0x14001;
        WStack_558 = (WORD)local_7d8;
        uStack_556 = (ushort)((ulonglong)local_7d8 >> 0x10);
        uStack_554 = (uint)((ulonglong)local_7d8 >> 0x20);
        uStack_550._0_1_ = 0xb0;
        uStack_550._1_1_ = 0xdb;
        uStack_550._2_1_ = 1;
        uStack_550._3_1_ = 0x40;
        uStack_550._4_4_ = 1;
        alloc::fmt::format::format_inner
                  (&local_908,(ushort *)&DAT_14015aabd,(ulonglong)&local_568);
        if (local_7d8 != (undefined1  [8])0x0) {
          __rustc::__rust_dealloc(pppppppuStack_7d0,local_7d8,1);
        }
        if (local_8e8._0_8_ != 0) {
          __rustc::__rust_dealloc(stack0xfffffffffffff720,local_8e8._0_8_,1);
        }
        local_7f8 = (undefined1 *)CONCAT71(local_908._1_7_,(byte)local_908);
        pcStack_7f0 = (code *)CONCAT71(uStack_8ff,uStack_900);
        local_7e8 = (LPCWSTR *)CONCAT71(uStack_8f7,uStack_8f8);
        lVar77 = 8;
        pbVar46 = <str>::trim_matches::<<char>::is_whitespace>(&DAT_14015aadd,8);
        local_568._0_1_ = (char)pbVar46;
        local_568._1_1_ = (undefined1)((ulonglong)pbVar46 >> 8);
        local_568._2_2_ = (WORD)((ulonglong)pbVar46 >> 0x10);
        local_568._4_2_ = (WORD)((ulonglong)pbVar46 >> 0x20);
        local_568._6_2_ = (WORD)((ulonglong)pbVar46 >> 0x30);
        uStack_560 = (ushort)(pbVar46 + lVar77);
        uStack_55e = (undefined6)((ulonglong)(pbVar46 + lVar77) >> 0x10);
        WStack_558 = 1;
        uStack_556 = 0;
        uStack_554 = 0;
        uStack_550._0_1_ = 0;
        uStack_550._1_1_ = 0;
        uStack_550._2_1_ = 0;
        uStack_550._3_1_ = 0;
        uStack_550._4_4_ = 0;
        <alloc::string::String_as_core::iter::traits::collect::FromIterator<alloc::string::String>>
        ::from_iter::
        <core::iter::adapters::map::Map<core::str::iter::Bytes,reach_launcher::package::percent::_closure_0_>>
                  ((ulonglong *)local_7d8,&local_568);
        stack0xfffffffffffff720 =
             (code *)&<alloc::string::String_as_core::fmt::Display>
                      ::fmt;
        local_8e8._0_8_ = local_7d8;
        alloc::fmt::format::format_inner
                  (&local_568,(ushort *)&DAT_14015aae5,(ulonglong)local_8e8);
        if (local_7d8 != (undefined1  [8])0x0) {
          __rustc::__rust_dealloc(pppppppuStack_7d0,local_7d8,1);
        }
        lVar77 = CONCAT26(local_568._6_2_,
                          CONCAT24(local_568._4_2_,
                                   CONCAT22(local_568._2_2_,
                                            CONCAT11(local_568._1_1_,(char)local_568))));
        pvVar62 = (void *)CONCAT62(uStack_55e,uStack_560);
        uVar41 = CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
        if ((ulonglong)((longlong)local_7f8 - (longlong)local_7e8) < uVar41) {
          <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
          <alloc::alloc::Global>
                    ((ulonglong *)&local_7f8,(ulonglong)local_7e8,uVar41,(LPVOID)0x1,1);
LAB_140018a26:
          ppWVar51 = local_7e8;
          memcpy(pcStack_7f0 + (longlong)local_7e8,pvVar62,uVar41);
        }
        else {
          ppWVar51 = local_7e8;
          if (uVar41 != 0) goto LAB_140018a26;
        }
        ppWVar51 = (LPCWSTR *)((longlong)ppWVar51 + uVar41);
        local_7e8 = ppWVar51;
        if (lVar77 != 0) {
          __rustc::__rust_dealloc(pvVar62,lVar77,1);
        }
        if (param_6 != 0) {
          local_568._0_1_ = (char)param_6;
          local_568._1_1_ = (undefined1)((ulonglong)param_6 >> 8);
          local_568._2_2_ = (WORD)((ulonglong)param_6 >> 0x10);
          local_568._4_2_ = (WORD)((ulonglong)param_6 >> 0x20);
          local_568._6_2_ = (WORD)((ulonglong)param_6 >> 0x30);
          uStack_560 = (ushort)(param_7 + param_6);
          uStack_55e = (undefined6)((ulonglong)(param_7 + param_6) >> 0x10);
          WStack_558 = 1;
          uStack_556 = 0;
          uStack_554 = 0;
          uStack_550._0_1_ = 0;
          uStack_550._1_1_ = 0;
          uStack_550._2_1_ = 0;
          uStack_550._3_1_ = 0;
          uStack_550._4_4_ = 0;
          <alloc::string::String_as_core::iter::traits::collect::FromIterator<alloc::string::String>>
          ::from_iter::
          <core::iter::adapters::map::Map<core::str::iter::Bytes,reach_launcher::package::percent::_closure_0_>>
                    ((ulonglong *)local_7d8,&local_568);
          stack0xfffffffffffff720 =
               (code *)&<alloc::string::String_as_core::fmt::Display>
                        ::fmt;
          local_8e8._0_8_ = local_7d8;
          alloc::fmt::format::format_inner
                    (&local_568,(ushort *)&DAT_14015aaf1,(ulonglong)local_8e8);
          if (local_7d8 != (undefined1  [8])0x0) {
            __rustc::__rust_dealloc(pppppppuStack_7d0,local_7d8,1);
          }
          lVar77 = CONCAT26(local_568._6_2_,
                            CONCAT24(local_568._4_2_,
                                     CONCAT22(local_568._2_2_,
                                              CONCAT11(local_568._1_1_,(char)local_568))));
          pvVar62 = (void *)CONCAT62(uStack_55e,uStack_560);
          uVar41 = CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
          if ((ulonglong)((longlong)local_7f8 - (longlong)ppWVar51) < uVar41) {
            <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
            <alloc::alloc::Global>
                      ((ulonglong *)&local_7f8,(ulonglong)ppWVar51,uVar41,(LPVOID)0x1,1);
            ppWVar51 = local_7e8;
LAB_140018b36:
            memcpy(pcStack_7f0 + (longlong)ppWVar51,pvVar62,uVar41);
          }
          else if (uVar41 != 0) goto LAB_140018b36;
          ppWVar51 = (LPCWSTR *)((longlong)ppWVar51 + uVar41);
          local_7e8 = ppWVar51;
          if (lVar77 != 0) {
            __rustc::__rust_dealloc(pvVar62,lVar77,1);
          }
        }
        <alloc::vec::Vec<alloc::string::String>as_alloc::vec::spec_from_iter_nested::SpecFromIterNested<alloc::string::String,core::iter::adapters::map::Map<core::slice::iter::Iter<&str>,reach_launcher::package::s::{closure#0}>>>
        ::from_iter((ulonglong *)&local_940,&PTR_s___loader_14015ab08,(undefined8 *)&DAT_14015ab18);
        std::env::current_exe(&local_568);
        lVar77 = CONCAT26(local_568._6_2_,
                          CONCAT24(local_568._4_2_,
                                   CONCAT22(local_568._2_2_,
                                            CONCAT11(local_568._1_1_,(char)local_568))));
        ppWVar64 = (LPCWSTR *)CONCAT62(uStack_55e,uStack_560);
        ppWVar45 = ppWVar64;
        if (lVar77 == -1) {
LAB_140018c61:
          uVar40 = (uint)ppWVar45 & 3;
          uVar41 = (ulonglong)uVar40;
          if ((1 < uVar40 - 2) && (uVar41 != 0)) {
            uStack_560 = (ushort)(undefined1 *)((longlong)ppWVar45 + -1);
            uStack_55e = (undefined6)((ulonglong)((longlong)ppWVar45 + -1) >> 0x10);
            local_568._0_1_ = '\x03';
            uVar41 = <core::io::error::CustomOwner_as_core::ops::drop::Drop>
                     ::drop((longlong *)&uStack_560);
          }
          ppWVar45 = (LPCWSTR *)0x1;
          pcVar47 = (code *)0x0;
          ppWVar86 = (LPCWSTR *)0x0;
        }
        else {
          local_908._0_1_ = (byte)uStack_560;
          local_908._1_7_ = (undefined7)((ulonglong)ppWVar64 >> 8);
          uStack_900 = (undefined1)WStack_558;
          uStack_8ff = (undefined7)(CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558)) >> 8);
          _local_8e8 = (_SYSTEMTIME)(ZEXT816(1) << 0x40);
          local_8d8 = (LPCWSTR *)0x0;
          local_7c8 = (LPCWSTR *)0x60000020;
          pppppppuStack_7d0 =
               (undefined8 *******)&PTR_<alloc::string::String>_14015acd8;
          local_7d8 = (undefined1  [8])local_8e8;
          uVar41 = <std::path::Display_as_core::fmt::Display>::
                   fmt((longlong *)&local_908,(undefined8 *)local_7d8);
          ppWVar86 = local_8d8;
          if ((char)uVar41 != '\0') {
            core::result::unwrap_failed
                      ("a Display implementation returned an error unexpectedly",0x37,&local_969,
                       &DAT_14015a408,&PTR_s__rustc_b940084d7eb6a299eb4bfeb8e_14015ad90);
            goto LAB_14001be41;
          }
          pcVar47 = (code *)local_8e8._0_8_;
          ppWVar45 = (LPCWSTR *)stack0xfffffffffffff720;
          if (lVar77 != 0) {
            uVar41 = __rustc::__rust_dealloc(ppWVar64,lVar77,1);
          }
          if (pcVar47 == (code *)0xffffffffffffffff) goto LAB_140018c61;
        }
        local_918 = (LPCWSTR *)CONCAT71((int7)(uVar41 >> 8),1);
        local_830 = local_1b8;
        local_2f8 = local_1b0;
        local_920 = pcVar47;
        local_1e8 = ppWVar45;
        local_120 = ppWVar86;
        start::_closure_7_((undefined8 *)local_8e8,local_1b8,local_1b0,
                           (undefined8 *******)&DAT_14015ab18,(undefined1 *)0x12);
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        ppWVar64 = (LPCWSTR *)__rustc::__rust_alloc(6,1);
        pcVar47 = pcStack_7f0;
        puVar76 = local_7f8;
        local_928 = (LPCWSTR)CONCAT71((int7)((ulonglong)ppWVar64 >> 8),ppWVar64 == (LPCWSTR *)0x0);
        if (ppWVar64 == (LPCWSTR *)0x0) {
          alloc::raw_vec::handle_error(1,6);
          goto LAB_14001be41;
        }
        *(undefined2 *)((longlong)ppWVar64 + 4) = 0x6567;
        *(undefined4 *)ppWVar64 = 0x61702d2d;
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        puVar71 = (undefined8 *)__rustc::__rust_alloc(8,1);
        if (puVar71 == (undefined8 *)0x0) {
          alloc::raw_vec::handle_error(1,8);
          goto LAB_14001be41;
        }
        *puVar71 = 0x776f6c6c6f662d2d;
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        pcVar60 = (char *)__rustc::__rust_alloc(10,1);
        if (pcVar60 == (char *)0x0) {
          alloc::raw_vec::handle_error(1,10);
          goto LAB_14001be41;
        }
        builtin_strncpy(pcVar60,"host probe",10);
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        puVar50 = (undefined4 *)__rustc::__rust_alloc(6,1);
        if (puVar50 == (undefined4 *)0x0) {
          alloc::raw_vec::handle_error(1,6);
          goto LAB_14001be41;
        }
        *(undefined2 *)(puVar50 + 1) = 0x656d;
        *puVar50 = 0x61672d2d;
        start::_closure_7_(&local_568,local_830,local_2f8,(undefined8 *******)&DAT_14015ab40,
                           (undefined1 *)0xe);
        local_7b8 = local_8d8;
        local_7c8 = (LPCWSTR *)local_8e8._0_8_;
        pcStack_7c0 = stack0xfffffffffffff720;
        local_738 = CONCAT26(local_568._6_2_,
                             CONCAT24(local_568._4_2_,
                                      CONCAT22(local_568._2_2_,
                                               CONCAT11(local_568._1_1_,(char)local_568))));
        puStack_730 = (undefined8 *)CONCAT62(uStack_55e,uStack_560);
        local_728 = CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
        pcStack_7b0 = (code *)&DAT_00000006;
        ppWStack_7a0 = (LPCWSTR *)&DAT_00000006;
        local_798 = puVar76;
        pcStack_790 = pcVar47;
        ppWStack_780 = (LPCWSTR *)&DAT_00000008;
        ppWStack_770 = (LPCWSTR *)&DAT_00000008;
        local_768 = 10;
        local_758 = 10;
        local_750 = (LPCWSTR)&DAT_00000006;
        local_740 = (LPCWSTR *)&DAT_00000006;
        _local_7d8 = (_SYSTEMTIME)(ZEXT816(7) << 0x40);
        pWVar49 = local_930;
        ppWStack_7a8 = ppWVar64;
        local_788 = ppWVar51;
        puStack_778 = puVar71;
        pcStack_760 = pcVar60;
        puStack_748 = puVar50;
        if ((ulonglong)((longlong)local_940 - (longlong)local_930) < 7) {
          <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
          <alloc::alloc::Global>
                    ((ulonglong *)&local_940,(ulonglong)local_930,7,&DAT_00000008,0x18);
          pWVar49 = local_930;
        }
        ppWVar51 = local_938;
        memcpy(&local_568,local_7d8,0xb8);
        lVar77 = CONCAT26(local_568._6_2_,
                          CONCAT24(local_568._4_2_,
                                   CONCAT22(local_568._2_2_,
                                            CONCAT11(local_568._1_1_,(char)local_568))));
        lVar87 = CONCAT62(uStack_55e,uStack_560);
        lVar61 = lVar87 - lVar77;
        if (lVar61 != 0) {
          memcpy(ppWVar51 + (longlong)pWVar49 * 3,&WStack_558 + lVar77 * 0xc,lVar61 * 0x18);
          pWVar49 = (LPCWSTR)((longlong)pWVar49 + (lVar87 - lVar77));
        }
        local_568._0_1_ = '\0';
        local_568._1_1_ = 0;
        local_568._2_2_ = 0;
        local_568._4_2_ = 0;
        local_568._6_2_ = 0;
        uStack_560 = 6;
        uStack_55e = 0;
        WStack_558 = 0xa632;
        uStack_556 = 0x4015;
        uStack_554 = 1;
        uStack_550._0_1_ = 4;
        uStack_550._1_1_ = 0;
        uStack_550._2_1_ = 0;
        uStack_550._3_1_ = 0;
        uStack_550._4_4_ = 0;
        local_548 = 0x78;
        uStack_547 = 0x14015a6;
        pcStack_540 = (code *)&DAT_00000005;
        local_538._8_8_ = 9;
        local_538._0_8_ = &DAT_14015aa95;
        local_528 = &DAT_14015aa9e;
        pcStack_520 = (code *)&DAT_00000008;
        local_518 = (LPCWSTR *)0x14015a880;
        ppWStack_510 = (LPCWSTR *)&DAT_00000004;
        local_508 = (undefined8 *)&DAT_14015aaa6;
        ppWStack_500 = (LPCWSTR *)&DAT_00000004;
        local_928 = local_250;
        local_910 = ppWStack_248;
        lVar77 = 0;
        local_930 = pWVar49;
        do {
          local_908._0_1_ = (byte)*(undefined8 *)((longlong)&WStack_558 + lVar77);
          local_908._1_7_ =
               (undefined7)((ulonglong)*(undefined8 *)((longlong)&WStack_558 + lVar77) >> 8);
          uStack_900 = (undefined1)*(undefined8 *)((longlong)&uStack_550 + lVar77);
          uStack_8ff = (undefined7)((ulonglong)*(undefined8 *)((longlong)&uStack_550 + lVar77) >> 8)
          ;
          stack0xfffffffffffff720 = <&str_as_core::fmt::Display>::fmt;
          local_8e8._0_8_ = &local_908;
          alloc::fmt::format::format_inner
                    ((ulonglong *)local_7d8,(ushort *)&DAT_14015ac92,(ulonglong)local_8e8);
          ppWVar51 = local_7c8;
          auVar78 = local_7d8;
          pppppppuVar30 = pppppppuStack_7d0;
          stack0xfffffffffffff720 = <&str_as_core::fmt::Display>::fmt;
          local_8e8._0_8_ = &local_908;
          alloc::fmt::format::format_inner
                    ((ulonglong *)local_7d8,(ushort *)&DAT_14015ac9d,(ulonglong)local_8e8);
          auVar29 = local_7d8;
          pppppppuVar73 = pppppppuStack_7d0;
          start::_closure_5_((undefined8 *)local_7d8,local_928,(size_t)local_910,pppppppuStack_7d0,
                             (undefined1 *)local_7c8);
          local_8b0 = local_7c8;
          local_8c0 = (code *)local_7d8;
          ppWStack_8b8 = (LPCWSTR *)pppppppuStack_7d0;
          local_8d8 = (LPCWSTR *)auVar78;
          local_8d0 = (code *)pppppppuVar30;
          local_8c8 = ppWVar51;
          _local_8e8 = (_SYSTEMTIME)(ZEXT816(2) << 0x40);
          if ((ulonglong)((longlong)local_940 - (longlong)local_930) < 2) {
            <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
            <alloc::alloc::Global>
                      ((ulonglong *)&local_940,(ulonglong)local_930,2,&DAT_00000008,0x18);
          }
          pWVar49 = local_930;
          uVar79 = local_8e8._0_8_;
          lVar87 = (longlong)stack0xfffffffffffff720;
          _local_7d8 = _local_8e8;
          ppWStack_7a8 = ppWStack_8b8;
          ppWStack_7a0 = local_8b0;
          local_7b8 = local_8c8;
          pcStack_7b0 = local_8c0;
          local_7c8 = local_8d8;
          pcStack_7c0 = local_8d0;
          if ((longlong)stack0xfffffffffffff720 - local_8e8._0_8_ != 0) {
            memcpy(local_938 + (longlong)local_930 * 3,&local_7c8 + local_8e8._0_8_ * 3,
                   ((longlong)stack0xfffffffffffff720 - local_8e8._0_8_) * 0x18);
            local_930 = (LPCWSTR)((longlong)pWVar49 + (lVar87 - uVar79));
          }
          if (auVar29 != (undefined1  [8])0x0) {
            __rustc::__rust_dealloc(pppppppuVar73,auVar29,1);
          }
          lVar77 = lVar77 + 0x10;
        } while (lVar77 != 0x60);
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        pcVar47 = (code *)__rustc::__rust_alloc(0xc,1);
        if (pcVar47 == (code *)0x0) {
          alloc::raw_vec::handle_error(1,0xc);
          goto LAB_14001be41;
        }
        *(undefined8 *)pcVar47 = 0x72657473616d2d2d;
        *(undefined4 *)(pcVar47 + 8) = 0x6578652d;
        start::_closure_7_(&local_568,local_830,local_2f8,(undefined8 *******)&DAT_14015ab5a,
                           &DAT_0000000d);
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        uVar79 = 0;
        pcVar59 = (code *)__rustc::__rust_alloc(0xd,1);
        if (pcVar59 == (code *)0x0) {
          alloc::raw_vec::handle_error(1,0xd);
          goto LAB_14001be41;
        }
        *(undefined8 *)(pcVar59 + 5) = 0x74726f702d726574;
        *(undefined8 *)pcVar59 = 0x72657473616d2d2d;
        ppWVar51 = (LPCWSTR *)local_7d8;
        pvVar62 = (void *)<u16>::_fmt((uint)CONCAT62((int6)((ulonglong)uVar79 >> 0x10),0x2db4),
                                      (longlong)ppWVar51,5);
        if ((longlong)ppWVar51 < 0) {
          lVar77 = 0;
LAB_140019350:
          alloc::raw_vec::handle_error(lVar77,ppWVar51);
          goto LAB_14001be41;
        }
        if (ppWVar51 == (LPCWSTR *)0x0) {
          puVar71 = (undefined8 *)0x1;
          pvVar63 = local_818;
        }
        else {
          __rustc::__rust_no_alloc_shim_is_unstable_v2();
          lVar77 = 1;
          puVar71 = (undefined8 *)__rustc::__rust_alloc((longlong)ppWVar51,1);
          pvVar63 = local_818;
          if (puVar71 == (undefined8 *)0x0) goto LAB_140019350;
          memcpy(puVar71,pvVar62,(size_t)ppWVar51);
        }
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        pcVar60 = (char *)__rustc::__rust_alloc(0xc,1);
        if (pcVar60 == (char *)0x0) {
          alloc::raw_vec::handle_error(1,0xc);
          goto LAB_14001be41;
        }
        builtin_strncpy(pcVar60,"--master-url",0xc);
        local_750 = local_870;
        puStack_748 = local_2c0;
        local_740 = local_2f0;
        pcStack_7b0 = (code *)CONCAT26(local_568._6_2_,
                                       CONCAT24(local_568._4_2_,
                                                CONCAT22(local_568._2_2_,
                                                         CONCAT11(local_568._1_1_,(char)local_568)))
                                      );
        ppWStack_7a8 = (LPCWSTR *)CONCAT62(uStack_55e,uStack_560);
        ppWStack_7a0 = (LPCWSTR *)CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
        local_7c8 = (LPCWSTR *)&DAT_0000000c;
        local_7b8 = (LPCWSTR *)&DAT_0000000c;
        local_798 = &DAT_0000000d;
        local_788 = (LPCWSTR *)&DAT_0000000d;
        _local_7d8 = (_SYSTEMTIME)(ZEXT816(6) << 0x40);
        local_768 = 0xc;
        local_758 = 0xc;
        pcStack_7c0 = pcVar47;
        pcStack_790 = pcVar59;
        ppWStack_780 = ppWVar51;
        puStack_778 = puVar71;
        ppWStack_770 = ppWVar51;
        pcStack_760 = pcVar60;
        if ((ulonglong)((longlong)local_940 - (longlong)local_930) < 6) {
          <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
          <alloc::alloc::Global>
                    ((ulonglong *)&local_940,(ulonglong)local_930,6,&DAT_00000008,0x18);
          pvVar63 = local_818;
        }
        sVar92 = local_820;
        pWVar49 = local_930;
        ppWVar51 = local_938;
        memcpy(&local_568,local_7d8,0xa0);
        lVar77 = CONCAT26(local_568._6_2_,
                          CONCAT24(local_568._4_2_,
                                   CONCAT22(local_568._2_2_,
                                            CONCAT11(local_568._1_1_,(char)local_568))));
        lVar87 = CONCAT62(uStack_55e,uStack_560);
        lVar61 = lVar87 - lVar77;
        if (lVar61 != 0) {
          memcpy(ppWVar51 + (longlong)pWVar49 * 3,&WStack_558 + lVar77 * 0xc,lVar61 * 0x18);
          pWVar49 = (LPCWSTR)((longlong)pWVar49 + (lVar87 - lVar77));
        }
        local_930 = pWVar49;
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        uVar79 = 0xd;
        pcVar47 = (code *)__rustc::__rust_alloc(0xd,1);
        if (pcVar47 == (code *)0x0) {
LAB_14001bce1:
          local_918 = (LPCWSTR *)0x0;
          alloc::raw_vec::handle_error(1,uVar79);
          goto LAB_14001be41;
        }
        *(undefined8 *)(pcVar47 + 5) = 0x6578652d72657377;
        *(undefined8 *)pcVar47 = 0x6573776f72622d2d;
        start::_closure_7_(&local_888,local_830,local_2f8,(undefined8 *******)&DAT_14015ab8d,
                           (undefined1 *)0xe);
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        pcVar59 = (code *)__rustc::__rust_alloc(0xc,1);
        if (pcVar59 == (code *)0x0) {
          alloc::raw_vec::handle_error(1,0xc);
          goto LAB_14001be41;
        }
        *(undefined8 *)pcVar59 = 0x6573776f72622d2d;
        *(undefined4 *)(pcVar59 + 8) = 0x69752d72;
        <std::path::Path>::_join
                  ((undefined8 *)local_8e8,pvVar63,sVar92,
                   (undefined8 *******)
                   &anon_261146bec65efbdf21e1c5186a43e7c0_17_llvm_12131923139401574060,&DAT_00000006
                  );
        pvVar62 = stack0xfffffffffffff720;
        <std::path::Path>::_join
                  (&local_908,stack0xfffffffffffff720,(size_t)local_8d8,
                   (undefined8 *******)&DAT_14015aaaa,&DAT_00000007);
        puVar44 = (undefined *)CONCAT71(uStack_8ff,uStack_900);
        local_298 = CONCAT71(uStack_8f7,uStack_8f8);
        local_8a8 = (LPCWSTR *)0x0;
        puStack_8a0 = (undefined8 *)0x1;
        local_898 = (LPCWSTR *)0x0;
        WStack_558 = 0x20;
        uStack_556 = 0x6000;
        uStack_554 = 0;
        local_568._0_1_ = (char)&local_8a8;
        local_568._1_1_ = (undefined1)((ulonglong)&local_8a8 >> 8);
        local_568._2_2_ = (WORD)((ulonglong)&local_8a8 >> 0x10);
        local_568._4_2_ = (WORD)((ulonglong)&local_8a8 >> 0x20);
        local_568._6_2_ = (WORD)((ulonglong)&local_8a8 >> 0x30);
        uStack_560 = 0xacd8;
        uStack_55e = 0x14015;
        local_2a0 = puVar44;
        cVar37 = <std::path::Display_as_core::fmt::Display>::fmt
                           ((longlong *)&local_2a0,&local_568);
        if (cVar37 != '\0') {
          core::result::unwrap_failed
                    ("a Display implementation returned an error unexpectedly",0x37,&local_969,
                     &DAT_14015a408,&PTR_s__rustc_b940084d7eb6a299eb4bfeb8e_14015ad90);
          goto LAB_14001be41;
        }
        ppWStack_770 = local_898;
        ppWStack_780 = local_8a8;
        puStack_778 = puStack_8a0;
        pcStack_7b0 = local_888;
        ppWStack_7a8 = ppWStack_880;
        ppWStack_7a0 = local_878;
        local_7c8 = (LPCWSTR *)&DAT_0000000d;
        local_7b8 = (LPCWSTR *)&DAT_0000000d;
        local_798 = &DAT_0000000c;
        local_788 = (LPCWSTR *)&DAT_0000000c;
        _local_7d8 = (_SYSTEMTIME)(ZEXT816(4) << 0x40);
        pcStack_7c0 = pcVar47;
        pcStack_790 = pcVar59;
        if ((ulonglong)((longlong)local_940 - (longlong)local_930) < 4) {
          <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
          <alloc::alloc::Global>
                    ((ulonglong *)&local_940,(ulonglong)local_930,4,&DAT_00000008,0x18);
        }
        pWVar49 = local_930;
        pppppppuVar30 = pppppppuStack_7d0;
        local_568._0_1_ = local_7d8[0];
        local_568._1_1_ = local_7d8[1];
        local_568._2_2_ = local_7d8._2_2_;
        local_568._4_2_ = local_7d8._4_2_;
        local_568._6_2_ = local_7d8._6_2_;
        uStack_560 = local_7d8._8_2_;
        uStack_55e = local_7d8._10_6_;
        local_508 = puStack_778;
        ppWStack_500 = ppWStack_770;
        local_518 = local_788;
        ppWStack_510 = ppWStack_780;
        local_528 = local_798;
        pcStack_520 = pcStack_790;
        local_538._8_8_ = ppWStack_7a0;
        local_538._0_8_ = ppWStack_7a8;
        local_548 = SUB81(local_7b8,0);
        uStack_547 = (undefined7)((ulonglong)local_7b8 >> 8);
        pcStack_540 = pcStack_7b0;
        WStack_558 = (WORD)local_7c8;
        uStack_556 = (ushort)((ulonglong)local_7c8 >> 0x10);
        uStack_554 = (uint)((ulonglong)local_7c8 >> 0x20);
        uStack_550._0_1_ = SUB81(pcStack_7c0,0);
        uStack_550._1_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 8);
        uStack_550._2_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 0x10);
        uStack_550._3_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 0x18);
        uStack_550._4_4_ = (undefined4)((ulonglong)pcStack_7c0 >> 0x20);
        lVar77 = local_7d8;
        if ((longlong)pppppppuStack_7d0 - lVar77 != 0) {
          memcpy(local_938 + (longlong)local_930 * 3,&WStack_558 + lVar77 * 0xc,
                 ((longlong)pppppppuStack_7d0 - lVar77) * 0x18);
          local_930 = (LPCWSTR)((longlong)pWVar49 + ((longlong)pppppppuVar30 - lVar77));
        }
        if (CONCAT71(local_908._1_7_,(byte)local_908) != 0) {
          __rustc::__rust_dealloc
                    (puVar44,CONCAT71(local_908._1_7_,(byte)local_908),1);
        }
        if (local_8e8._0_8_ != 0) {
          __rustc::__rust_dealloc(pvVar62,local_8e8._0_8_,1);
        }
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        uVar79 = 7;
        pcVar47 = (code *)__rustc::__rust_alloc(7,1);
        if (pcVar47 == (code *)0x0) goto LAB_14001bce1;
        *(undefined4 *)(pcVar47 + 3) = 0x656e6563;
        *(undefined4 *)pcVar47 = 0x63732d2d;
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        ppWVar51 = (LPCWSTR *)__rustc::__rust_alloc(5,1);
        if (ppWVar51 == (LPCWSTR *)0x0) {
          alloc::raw_vec::handle_error(1,5);
          goto LAB_14001be41;
        }
        *(undefined1 *)((longlong)ppWVar51 + 4) = 0x65;
        *(undefined4 *)ppWVar51 = 0x6c746974;
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        pcVar59 = (code *)__rustc::__rust_alloc(0xb,1);
        if (pcVar59 == (code *)0x0) {
          alloc::raw_vec::handle_error(1,0xb);
          goto LAB_14001be41;
        }
        *(undefined8 *)pcVar59 = 0x642d726573752d2d;
        *(undefined4 *)(pcVar59 + 7) = 0x61746164;
        local_2f0 = local_1d8;
        start::_closure_4_(&local_568,local_1d8,(size_t)local_1d0,(undefined8 *******)&DAT_14015abbe
                           ,&DAT_00000007);
        ppWStack_770 = (LPCWSTR *)CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
        ppWStack_780 = (LPCWSTR *)
                       CONCAT26(local_568._6_2_,
                                CONCAT24(local_568._4_2_,
                                         CONCAT22(local_568._2_2_,
                                                  CONCAT11(local_568._1_1_,(char)local_568))));
        puStack_778 = (undefined8 *)CONCAT62(uStack_55e,uStack_560);
        local_7c8 = (LPCWSTR *)&DAT_00000007;
        local_7b8 = (LPCWSTR *)&DAT_00000007;
        pcStack_7b0 = (code *)&DAT_00000005;
        ppWStack_7a0 = (LPCWSTR *)&DAT_00000005;
        local_798 = (undefined1 *)0xb;
        local_788 = (LPCWSTR *)0xb;
        _local_7d8 = (_SYSTEMTIME)(ZEXT816(4) << 0x40);
        local_918 = local_1d0;
        pcStack_7c0 = pcVar47;
        ppWStack_7a8 = ppWVar51;
        pcStack_790 = pcVar59;
        if ((ulonglong)((longlong)local_940 - (longlong)local_930) < 4) {
          <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
          <alloc::alloc::Global>
                    ((ulonglong *)&local_940,(ulonglong)local_930,4,&DAT_00000008,0x18);
        }
        ppWVar51 = local_918;
        pWVar49 = local_930;
        pppppppuVar30 = pppppppuStack_7d0;
        local_568._0_1_ = local_7d8[0];
        local_568._1_1_ = local_7d8[1];
        local_568._2_2_ = local_7d8._2_2_;
        local_568._4_2_ = local_7d8._4_2_;
        local_568._6_2_ = local_7d8._6_2_;
        uStack_560 = local_7d8._8_2_;
        uStack_55e = local_7d8._10_6_;
        local_508 = puStack_778;
        ppWStack_500 = ppWStack_770;
        local_518 = local_788;
        ppWStack_510 = ppWStack_780;
        local_528 = local_798;
        pcStack_520 = pcStack_790;
        local_538._8_8_ = ppWStack_7a0;
        local_538._0_8_ = ppWStack_7a8;
        local_548 = SUB81(local_7b8,0);
        uStack_547 = (undefined7)((ulonglong)local_7b8 >> 8);
        pcStack_540 = pcStack_7b0;
        WStack_558 = (WORD)local_7c8;
        uStack_556 = (ushort)((ulonglong)local_7c8 >> 0x10);
        uStack_554 = (uint)((ulonglong)local_7c8 >> 0x20);
        uStack_550._0_1_ = SUB81(pcStack_7c0,0);
        uStack_550._1_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 8);
        uStack_550._2_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 0x10);
        uStack_550._3_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 0x18);
        uStack_550._4_4_ = (undefined4)((ulonglong)pcStack_7c0 >> 0x20);
        lVar77 = local_7d8;
        if ((longlong)pppppppuStack_7d0 - lVar77 != 0) {
          memcpy(local_938 + (longlong)local_930 * 3,&WStack_558 + lVar77 * 0xc,
                 ((longlong)pppppppuStack_7d0 - lVar77) * 0x18);
          local_930 = (LPCWSTR)((longlong)pWVar49 + ((longlong)pppppppuVar30 - lVar77));
        }
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        uVar79 = 0xe;
        pcVar47 = (code *)__rustc::__rust_alloc(0xe,1);
        ppWVar64 = ppWStack_218;
        if (pcVar47 == (code *)0x0) goto LAB_14001bce1;
        *(undefined8 *)(pcVar47 + 6) = 0x657361622d656c69;
        *(undefined8 *)pcVar47 = 0x6c69666f72702d2d;
        start::_closure_3_(&local_908,ppWStack_218,local_210,(undefined8 *******)&DAT_14015abd3,
                           &DAT_0000000d);
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        pcVar59 = (code *)__rustc::__rust_alloc(0xd,1);
        if (pcVar59 == (code *)0x0) {
          alloc::raw_vec::handle_error(1,0xd);
          goto LAB_14001be41;
        }
        *(undefined8 *)(pcVar59 + 5) = 0x74756f2d656c6966;
        *(undefined8 *)pcVar59 = 0x6c69666f72702d2d;
        start::_closure_5_((undefined8 *)local_8e8,local_928,(size_t)local_910,
                           (undefined8 *******)"profile.bin--strings",(undefined1 *)0xb);
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        pcVar60 = (char *)__rustc::__rust_alloc(0xc,1);
        if (pcVar60 == (char *)0x0) {
          alloc::raw_vec::handle_error(1,0xc);
          goto LAB_14001be41;
        }
        builtin_strncpy(pcVar60,"--armor-file",0xc);
        start::_closure_4_(&local_568,local_2f0,(size_t)ppWVar51,(undefined8 *******)&DAT_14015abf9,
                           &DAT_00000009);
        ppWStack_7a0 = (LPCWSTR *)CONCAT71(uStack_8f7,uStack_8f8);
        pcStack_7b0 = (code *)CONCAT71(local_908._1_7_,(byte)local_908);
        ppWStack_7a8 = (LPCWSTR *)CONCAT71(uStack_8ff,uStack_900);
        ppWStack_780 = (LPCWSTR *)local_8e8._0_8_;
        puStack_778 = (undefined8 *)stack0xfffffffffffff720;
        ppWStack_770 = local_8d8;
        local_750 = (LPCWSTR)CONCAT26(local_568._6_2_,
                                      CONCAT24(local_568._4_2_,
                                               CONCAT22(local_568._2_2_,
                                                        CONCAT11(local_568._1_1_,(char)local_568))))
        ;
        puStack_748 = (undefined4 *)CONCAT62(uStack_55e,uStack_560);
        local_740 = (LPCWSTR *)CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
        local_7c8 = (LPCWSTR *)0xe;
        local_7b8 = (LPCWSTR *)0xe;
        local_798 = &DAT_0000000d;
        local_788 = (LPCWSTR *)&DAT_0000000d;
        local_768 = 0xc;
        local_758 = 0xc;
        _local_7d8 = (_SYSTEMTIME)(ZEXT816(6) << 0x40);
        local_128 = ppWVar64;
        pcStack_7c0 = pcVar47;
        pcStack_790 = pcVar59;
        pcStack_760 = pcVar60;
        if ((ulonglong)((longlong)local_940 - (longlong)local_930) < 6) {
          <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
          <alloc::alloc::Global>
                    ((ulonglong *)&local_940,(ulonglong)local_930,6,&DAT_00000008,0x18);
        }
        pWVar49 = local_930;
        ppWVar51 = local_938;
        memcpy(&local_568,local_7d8,0xa0);
        lVar77 = CONCAT26(local_568._6_2_,
                          CONCAT24(local_568._4_2_,
                                   CONCAT22(local_568._2_2_,
                                            CONCAT11(local_568._1_1_,(char)local_568))));
        lVar87 = CONCAT62(uStack_55e,uStack_560);
        lVar61 = lVar87 - lVar77;
        if (lVar61 != 0) {
          memcpy(ppWVar51 + (longlong)pWVar49 * 3,&WStack_558 + lVar77 * 0xc,lVar61 * 0x18);
          pWVar49 = (LPCWSTR)((longlong)pWVar49 + (lVar87 - lVar77));
        }
        local_930 = pWVar49;
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        uVar79 = 10;
        pcVar47 = (code *)__rustc::__rust_alloc(10,1);
        if (pcVar47 == (code *)0x0) goto LAB_14001bce1;
        *(undefined8 *)pcVar47 = 0x68636e75616c2d2d;
        *(undefined2 *)(pcVar47 + 8) = 0x7265;
        pcStack_7b0 = local_920;
        ppWStack_7a8 = local_1e8;
        ppWStack_7a0 = local_120;
        _local_7d8 = (_SYSTEMTIME)(ZEXT816(2) << 0x40);
        local_7c8 = (LPCWSTR *)&DAT_0000000a;
        local_7b8 = (LPCWSTR *)&DAT_0000000a;
        pcStack_7c0 = pcVar47;
        if ((ulonglong)((longlong)local_940 - (longlong)local_930) < 2) {
          <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
          <alloc::alloc::Global>
                    ((ulonglong *)&local_940,(ulonglong)local_930,2,&DAT_00000008,0x18);
        }
        pWVar49 = local_930;
        pppppppuVar30 = pppppppuStack_7d0;
        local_568._0_1_ = local_7d8[0];
        local_568._1_1_ = local_7d8[1];
        local_568._2_2_ = local_7d8._2_2_;
        local_568._4_2_ = local_7d8._4_2_;
        local_568._6_2_ = local_7d8._6_2_;
        uStack_560 = local_7d8._8_2_;
        uStack_55e = local_7d8._10_6_;
        local_538._8_8_ = ppWStack_7a0;
        local_538._0_8_ = ppWStack_7a8;
        local_548 = SUB81(local_7b8,0);
        uStack_547 = (undefined7)((ulonglong)local_7b8 >> 8);
        pcStack_540 = pcStack_7b0;
        WStack_558 = (WORD)local_7c8;
        uStack_556 = (ushort)((ulonglong)local_7c8 >> 0x10);
        uStack_554 = (uint)((ulonglong)local_7c8 >> 0x20);
        uStack_550._0_1_ = SUB81(pcStack_7c0,0);
        uStack_550._1_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 8);
        uStack_550._2_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 0x10);
        uStack_550._3_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 0x18);
        uStack_550._4_4_ = (undefined4)((ulonglong)pcStack_7c0 >> 0x20);
        lVar77 = local_7d8;
        if ((longlong)pppppppuStack_7d0 - lVar77 != 0) {
          memcpy(local_938 + (longlong)local_930 * 3,&WStack_558 + lVar77 * 0xc,
                 ((longlong)pppppppuStack_7d0 - lVar77) * 0x18);
          local_930 = (LPCWSTR)((longlong)pWVar49 + ((longlong)pppppppuVar30 - lVar77));
        }
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        pcVar59 = (code *)__rustc::__rust_alloc(0x11,1);
        ppWVar51 = local_1f8;
        pcVar47 = local_200;
        if (pcVar59 == (code *)0x0) {
          alloc::raw_vec::handle_error(1,0x11);
          goto LAB_14001be41;
        }
        *(undefined8 *)pcVar59 = 0x7261657070612d2d;
        *(undefined8 *)(pcVar59 + 8) = 0x6c69662d65636e61;
        pcVar59[0x10] = (code)0x65;
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        pcVar65 = (code *)__rustc::__rust_alloc(0xf,1);
        pcVar27 = local_160;
        if (pcVar65 == (code *)0x0) {
          alloc::raw_vec::handle_error(1,0xf);
          goto LAB_14001be41;
        }
        *(undefined8 *)(pcVar65 + 7) = 0x656c69662d73676e;
        *(undefined8 *)pcVar65 = 0x6e69747465732d2d;
        stack0xfffffffffffff720 = local_158;
        local_8e8._0_8_ = local_160;
        _local_7d8 = (_SYSTEMTIME)(ZEXT816(1) << 0x40);
        local_7c8 = (LPCWSTR *)0x0;
        WStack_558 = 0x20;
        uStack_556 = 0x6000;
        uStack_554 = 0;
        local_568._0_1_ = (char)local_7d8;
        local_568._1_1_ = (undefined1)((ulonglong)local_7d8 >> 8);
        local_568._2_2_ = (WORD)((ulonglong)local_7d8 >> 0x10);
        local_568._4_2_ = (WORD)((ulonglong)local_7d8 >> 0x20);
        local_568._6_2_ = (WORD)((ulonglong)local_7d8 >> 0x30);
        uStack_560 = 0xacd8;
        uStack_55e = 0x14015;
        cVar37 = <std::path::Display_as_core::fmt::Display>::fmt
                           ((longlong *)local_8e8,&local_568);
        ppWVar64 = local_7c8;
        local_920 = pcVar27;
        if (cVar37 != '\0') {
          core::result::unwrap_failed
                    ("a Display implementation returned an error unexpectedly",0x37,&local_969,
                     &DAT_14015a408,&PTR_s__rustc_b940084d7eb6a299eb4bfeb8e_14015ad90);
          goto LAB_14001be41;
        }
        auVar78 = local_7d8;
        pppppppuVar30 = pppppppuStack_7d0;
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        pcVar60 = (char *)__rustc::__rust_alloc(0xc,1);
        if (pcVar60 == (char *)0x0) {
          alloc::raw_vec::handle_error(1,0xc);
          goto LAB_14001be41;
        }
        builtin_strncpy(pcVar60,"--scene-file",0xc);
        local_740 = local_170;
        local_750 = local_180;
        puStack_748 = puStack_178;
        local_7c8 = (LPCWSTR *)&DAT_00000011;
        local_7b8 = (LPCWSTR *)&DAT_00000011;
        pcStack_7b0 = pcVar47;
        ppWStack_7a8 = ppWVar51;
        ppWStack_7a0 = local_1f0;
        local_798 = &DAT_0000000f;
        local_788 = (LPCWSTR *)&DAT_0000000f;
        ppWStack_780 = (LPCWSTR *)auVar78;
        puStack_778 = pppppppuVar30;
        ppWStack_770 = ppWVar64;
        _local_7d8 = (_SYSTEMTIME)(ZEXT816(6) << 0x40);
        local_768 = 0xc;
        local_758 = 0xc;
        pcStack_7c0 = pcVar59;
        pcStack_790 = pcVar65;
        pcStack_760 = pcVar60;
        if ((ulonglong)((longlong)local_940 - (longlong)local_930) < 6) {
          <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
          <alloc::alloc::Global>
                    ((ulonglong *)&local_940,(ulonglong)local_930,6,&DAT_00000008,0x18);
        }
        hMutex = local_828;
        param_1 = local_850;
        pWVar49 = local_930;
        ppWVar51 = local_938;
        memcpy(&local_568,local_7d8,0xa0);
        lVar77 = CONCAT26(local_568._6_2_,
                          CONCAT24(local_568._4_2_,
                                   CONCAT22(local_568._2_2_,
                                            CONCAT11(local_568._1_1_,(char)local_568))));
        lVar87 = CONCAT62(uStack_55e,uStack_560);
        lVar61 = lVar87 - lVar77;
        if (lVar61 != 0) {
          memcpy(ppWVar51 + (longlong)pWVar49 * 3,&WStack_558 + lVar77 * 0xc,lVar61 * 0x18);
          pWVar49 = (LPCWSTR)((longlong)pWVar49 + (lVar87 - lVar77));
        }
        local_930 = pWVar49;
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        uVar79 = 5;
        pcVar47 = (code *)__rustc::__rust_alloc(5,1);
        if (pcVar47 == (code *)0x0) goto LAB_14001bd01;
        pcVar47[4] = (code)0x63;
        *(undefined4 *)pcVar47 = 0x636d2d2d;
        local_2a0 = local_2e0;
        local_298 = param_5;
        _local_7d8 = (_SYSTEMTIME)(ZEXT816(1) << 0x40);
        local_7c8 = (LPCWSTR *)0x0;
        WStack_558 = 0x20;
        uStack_556 = 0x6000;
        uStack_554 = 0;
        local_568._0_1_ = (char)local_7d8;
        local_568._1_1_ = (undefined1)((ulonglong)local_7d8 >> 8);
        local_568._2_2_ = (WORD)((ulonglong)local_7d8 >> 0x10);
        local_568._4_2_ = (WORD)((ulonglong)local_7d8 >> 0x20);
        local_568._6_2_ = (WORD)((ulonglong)local_7d8 >> 0x30);
        uStack_560 = 0xacd8;
        uStack_55e = 0x14015;
        cVar37 = <std::path::Display_as_core::fmt::Display>::fmt
                           ((longlong *)&local_2a0,&local_568);
        ppWVar51 = local_7c8;
        if (cVar37 != '\0') {
          core::result::unwrap_failed
                    ("a Display implementation returned an error unexpectedly",0x37,&local_969,
                     &DAT_14015a408,&PTR_s__rustc_b940084d7eb6a299eb4bfeb8e_14015ad90);
          goto LAB_14001be41;
        }
        auVar78 = local_7d8;
        pppppppuVar30 = pppppppuStack_7d0;
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        pcVar59 = (code *)__rustc::__rust_alloc(0xb,1);
        if (pcVar59 == (code *)0x0) {
          alloc::raw_vec::handle_error(1,0xb);
          goto LAB_14001be41;
        }
        *(undefined8 *)pcVar59 = 0x7274736d67762d2d;
        *(undefined4 *)(pcVar59 + 7) = 0x6d616572;
        <std::path::Path>::_join
                  ((undefined8 *)local_8e8,local_830,local_2f8,(undefined8 *******)&DAT_14015ac43,
                   &DAT_00000009);
        local_2e0 = stack0xfffffffffffff720;
        <std::path::Path>::_join
                  (&local_908,stack0xfffffffffffff720,(size_t)local_8d8,
                   (undefined8 *******)&DAT_14015ac4c,&DAT_00000011);
        pcVar27 = (code *)CONCAT71(uStack_8ff,uStack_900);
        ppWStack_880 = (LPCWSTR *)CONCAT71(uStack_8f7,uStack_8f8);
        local_8a8 = (LPCWSTR *)0x0;
        puStack_8a0 = (undefined8 *)0x1;
        local_898 = (LPCWSTR *)0x0;
        WStack_558 = 0x20;
        uStack_556 = 0x6000;
        uStack_554 = 0;
        local_568._0_1_ = (char)&local_8a8;
        local_568._1_1_ = (undefined1)((ulonglong)&local_8a8 >> 8);
        local_568._2_2_ = (WORD)((ulonglong)&local_8a8 >> 0x10);
        local_568._4_2_ = (WORD)((ulonglong)&local_8a8 >> 0x20);
        local_568._6_2_ = (WORD)((ulonglong)&local_8a8 >> 0x30);
        uStack_560 = 0xacd8;
        uStack_55e = 0x14015;
        local_888 = pcVar27;
        cVar37 = <std::path::Display_as_core::fmt::Display>::fmt
                           ((longlong *)&local_888,&local_568);
        if (cVar37 != '\0') {
          core::result::unwrap_failed
                    ("a Display implementation returned an error unexpectedly",0x37,&local_969,
                     &DAT_14015a408,&PTR_s__rustc_b940084d7eb6a299eb4bfeb8e_14015ad90);
          goto LAB_14001be41;
        }
        ppWStack_770 = local_898;
        ppWStack_780 = local_8a8;
        puStack_778 = puStack_8a0;
        local_7c8 = (LPCWSTR *)&DAT_00000005;
        local_7b8 = (LPCWSTR *)&DAT_00000005;
        pcStack_7b0 = (code *)auVar78;
        ppWStack_7a8 = (LPCWSTR *)pppppppuVar30;
        ppWStack_7a0 = ppWVar51;
        local_798 = (undefined1 *)0xb;
        local_788 = (LPCWSTR *)0xb;
        _local_7d8 = (_SYSTEMTIME)(ZEXT816(4) << 0x40);
        pcStack_7c0 = pcVar47;
        pcStack_790 = pcVar59;
        if ((ulonglong)((longlong)local_940 - (longlong)local_930) < 4) {
          <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
          <alloc::alloc::Global>
                    ((ulonglong *)&local_940,(ulonglong)local_930,4,&DAT_00000008,0x18);
          hMutex = local_828;
          param_1 = local_850;
        }
        pWVar49 = local_930;
        pppppppuVar30 = pppppppuStack_7d0;
        local_568._0_1_ = local_7d8[0];
        local_568._1_1_ = local_7d8[1];
        local_568._2_2_ = local_7d8._2_2_;
        local_568._4_2_ = local_7d8._4_2_;
        local_568._6_2_ = local_7d8._6_2_;
        uStack_560 = local_7d8._8_2_;
        uStack_55e = local_7d8._10_6_;
        local_508 = puStack_778;
        ppWStack_500 = ppWStack_770;
        local_518 = local_788;
        ppWStack_510 = ppWStack_780;
        local_528 = local_798;
        pcStack_520 = pcStack_790;
        local_538._8_8_ = ppWStack_7a0;
        local_538._0_8_ = ppWStack_7a8;
        local_548 = SUB81(local_7b8,0);
        uStack_547 = (undefined7)((ulonglong)local_7b8 >> 8);
        pcStack_540 = pcStack_7b0;
        WStack_558 = (WORD)local_7c8;
        uStack_556 = (ushort)((ulonglong)local_7c8 >> 0x10);
        uStack_554 = (uint)((ulonglong)local_7c8 >> 0x20);
        uStack_550._0_1_ = SUB81(pcStack_7c0,0);
        uStack_550._1_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 8);
        uStack_550._2_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 0x10);
        uStack_550._3_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 0x18);
        uStack_550._4_4_ = (undefined4)((ulonglong)pcStack_7c0 >> 0x20);
        lVar77 = local_7d8;
        if ((longlong)pppppppuStack_7d0 - lVar77 != 0) {
          memcpy(local_938 + (longlong)local_930 * 3,&WStack_558 + lVar77 * 0xc,
                 ((longlong)pppppppuStack_7d0 - lVar77) * 0x18);
          local_930 = (LPCWSTR)((longlong)pWVar49 + ((longlong)pppppppuVar30 - lVar77));
        }
        if (CONCAT71(local_908._1_7_,(byte)local_908) != 0) {
          __rustc::__rust_dealloc
                    (pcVar27,CONCAT71(local_908._1_7_,(byte)local_908),1);
        }
        if (local_8e8._0_8_ != 0) {
          __rustc::__rust_dealloc(local_2e0,local_8e8._0_8_,1);
        }
        __rustc::__rust_no_alloc_shim_is_unstable_v2();
        uVar79 = 0xd;
        pcVar47 = (code *)__rustc::__rust_alloc(0xd,1);
        if (pcVar47 != (code *)0x0) {
          *(undefined8 *)(pcVar47 + 5) = 0x65686361632d6369;
          *(undefined8 *)pcVar47 = 0x2d636973756d2d2d;
          start::_closure_4_(&local_568,local_2f0,(size_t)local_918,
                             (undefined8 *******)&DAT_14015ac6a,&DAT_00000005);
          ppWStack_7a0 = (LPCWSTR *)CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
          pcStack_7b0 = (code *)CONCAT26(local_568._6_2_,
                                         CONCAT24(local_568._4_2_,
                                                  CONCAT22(local_568._2_2_,
                                                           CONCAT11(local_568._1_1_,(char)local_568)
                                                          )));
          ppWStack_7a8 = (LPCWSTR *)CONCAT62(uStack_55e,uStack_560);
          local_7c8 = (LPCWSTR *)&DAT_0000000d;
          local_7b8 = (LPCWSTR *)&DAT_0000000d;
          _local_7d8 = (_SYSTEMTIME)(ZEXT816(2) << 0x40);
          pcStack_7c0 = pcVar47;
          if ((ulonglong)((longlong)local_940 - (longlong)local_930) < 2) {
            <alloc::raw_vec::RawVecInner<_>>::reserve::do_reserve_and_handle::
            <alloc::alloc::Global>
                      ((ulonglong *)&local_940,(ulonglong)local_930,2,&DAT_00000008,0x18);
            hMutex = local_828;
            param_1 = local_850;
          }
          pWVar49 = local_930;
          pppppppuVar30 = pppppppuStack_7d0;
          local_568._0_1_ = local_7d8[0];
          local_568._1_1_ = local_7d8[1];
          local_568._2_2_ = local_7d8._2_2_;
          local_568._4_2_ = local_7d8._4_2_;
          local_568._6_2_ = local_7d8._6_2_;
          uStack_560 = local_7d8._8_2_;
          uStack_55e = local_7d8._10_6_;
          local_538._8_8_ = ppWStack_7a0;
          local_538._0_8_ = ppWStack_7a8;
          local_548 = SUB81(local_7b8,0);
          uStack_547 = (undefined7)((ulonglong)local_7b8 >> 8);
          pcStack_540 = pcStack_7b0;
          WStack_558 = (WORD)local_7c8;
          uStack_556 = (ushort)((ulonglong)local_7c8 >> 0x10);
          uStack_554 = (uint)((ulonglong)local_7c8 >> 0x20);
          uStack_550._0_1_ = SUB81(pcStack_7c0,0);
          uStack_550._1_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 8);
          uStack_550._2_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 0x10);
          uStack_550._3_1_ = (undefined1)((ulonglong)pcStack_7c0 >> 0x18);
          uStack_550._4_4_ = (undefined4)((ulonglong)pcStack_7c0 >> 0x20);
          lVar77 = local_7d8;
          if ((longlong)pppppppuStack_7d0 - lVar77 != 0) {
            memcpy(local_938 + (longlong)local_930 * 3,&WStack_558 + lVar77 * 0xc,
                   ((longlong)pppppppuStack_7d0 - lVar77) * 0x18);
            local_930 = (LPCWSTR)((longlong)pWVar49 + ((longlong)pppppppuVar30 - lVar77));
          }
          __rustc::__rust_no_alloc_shim_is_unstable_v2();
          uVar79 = 0xd;
          pWVar42 = (LPCWSTR)__rustc::__rust_alloc(0xd,1);
          pWVar49 = local_930;
          if (pWVar42 != (LPCWSTR)0x0) {
            *(undefined8 *)((longlong)pWVar42 + 5) = 0x746168632d656d61;
            builtin_memcpy(pWVar42,L"â´­æ¹©æ…§æ•­",8);
            if (local_930 == local_940) {
              <alloc::raw_vec::RawVec<alloc::string::String>>::
              grow_one((ulonglong *)&local_940);
            }
            local_938[(longlong)pWVar49 * 3] = (LPCWSTR)0xd;
            local_938[(longlong)pWVar49 * 3 + 1] = pWVar42;
            local_938[(longlong)pWVar49 * 3 + 2] = (LPCWSTR)0xd;
            local_930 = (LPCWSTR)((longlong)pWVar49 + 1);
            <std::path::Path>::_join
                      ((undefined8 *)local_7d8,local_928,(size_t)local_910,
                       (undefined8 *******)&DAT_14015ac7c,(undefined1 *)0xb);
            local_568._0_1_ = '\0';
            local_568._1_1_ = 0;
            local_568._2_2_ = 0;
            uStack_560 = 0;
            uStack_55e = 0;
            WStack_558 = 7;
            uStack_556 = 0;
            uStack_554 = 0;
            uStack_550._0_1_ = 0;
            uStack_550._2_1_ = 0;
            local_548 = 0;
            uStack_550._1_1_ = 1;
            uStack_550._4_4_ = 1;
            uStack_550._3_1_ = 1;
            pppppppuVar30 = pppppppuStack_7d0;
            pppppppuVar73 = pppppppuStack_7d0;
            lVar87 = <std::fs::OpenOptions>::_open
                               ((int *)&local_568,(longlong)pppppppuStack_7d0,(longlong)local_7c8);
            lVar77 = local_960;
            ppWVar51 = local_968;
            if (local_7d8 != (undefined1  [8])0x0) {
              __rustc::__rust_dealloc(pppppppuVar30,local_7d8,1);
            }
            if (lVar87 == 1) {
              _local_7d8 = (_SYSTEMTIME)(ZEXT816(1) << 0x40);
              local_7c8 = (LPCWSTR *)0x0;
              WStack_558 = 0x20;
              uStack_556 = 0x6000;
              uStack_554 = 0;
              local_568._0_1_ = (char)local_7d8;
              local_568._1_1_ = (undefined1)((ulonglong)local_7d8 >> 8);
              local_568._2_2_ = (WORD)((ulonglong)local_7d8 >> 0x10);
              local_568._4_2_ = (WORD)((ulonglong)local_7d8 >> 0x20);
              local_568._6_2_ = (WORD)((ulonglong)local_7d8 >> 0x30);
              uStack_560 = 0xacd8;
              uStack_55e = 0x14015;
              local_8a8 = (LPCWSTR *)pppppppuVar73;
              cVar37 = <core::io::error::Error_as_core::fmt::Display>
                       ::fmt((ulonglong *)&local_8a8,&local_568);
              if (cVar37 != '\0') {
                core::result::unwrap_failed
                          ("a Display implementation returned an error unexpectedly",0x37,&local_969
                           ,&DAT_14015a408,&PTR_s__rustc_b940084d7eb6a299eb4bfeb8e_14015ad90);
                goto LAB_14001be41;
              }
              local_8d8 = local_7c8;
              _local_8e8 = _local_7d8;
              local_908._0_1_ = (byte)local_928;
              local_908._1_7_ = (undefined7)((ulonglong)local_928 >> 8);
              uStack_900 = SUB81(local_910,0);
              uStack_8ff = (undefined7)((ulonglong)local_910 >> 8);
              local_568._0_1_ = (char)local_8e8;
              local_568._1_1_ = (undefined1)((ulonglong)local_8e8 >> 8);
              local_568._2_2_ = (WORD)((ulonglong)local_8e8 >> 0x10);
              local_568._4_2_ = (WORD)((ulonglong)local_8e8 >> 0x20);
              local_568._6_2_ = (WORD)((ulonglong)local_8e8 >> 0x30);
              uStack_560 = 0xdbb0;
              uStack_55e = 0x14001;
              WStack_558 = (WORD)&local_908;
              uStack_556 = (ushort)((ulonglong)&local_908 >> 0x10);
              uStack_554 = (uint)((ulonglong)&local_908 >> 0x20);
              uStack_550._0_1_ = 0xe0;
              uStack_550._1_1_ = 0;
              uStack_550._2_1_ = 0x10;
              uStack_550._3_1_ = 0x40;
              uStack_550._4_4_ = 1;
              alloc::fmt::format::format_inner
                        ((ulonglong *)local_7d8,(ushort *)&DAT_14015a3d7,(ulonglong)&local_568);
              if (local_8e8._0_8_ != 0) {
                __rustc::__rust_dealloc(stack0xfffffffffffff720,local_8e8._0_8_,1)
                ;
              }
              if ((1 < ((uint)pppppppuVar73 & 3) - 2) && (((ulonglong)pppppppuVar73 & 3) != 0)) {
                uStack_560 = (ushort)(undefined1 *)((longlong)pppppppuVar73 + -1);
                uStack_55e = (undefined6)((ulonglong)((longlong)pppppppuVar73 + -1) >> 0x10);
                local_568._0_1_ = '\x03';
                <core::io::error::CustomOwner_as_core::ops::drop::Drop>
                ::drop((longlong *)&uStack_560);
              }
              *param_1 = (ulonglong)local_7d8;
              param_1[1] = (ulonglong)pppppppuStack_7d0;
              param_1[2] = (ulonglong)local_7c8;
              ppWVar51 = local_968;
              lVar77 = local_960;
LAB_14001aacf:
              pWVar49 = local_870;
              puVar71 = local_958;
              if (local_930 != (LPCWSTR)0x0) {
                ppWVar64 = local_938 + 1;
                pWVar42 = local_930;
                do {
                  if (ppWVar64[-1] != (LPCWSTR)0x0) {
                    __rustc::__rust_dealloc(*ppWVar64,ppWVar64[-1],1);
                    puVar71 = local_958;
                  }
                  ppWVar64 = ppWVar64 + 3;
                  pWVar42 = (LPCWSTR)((longlong)pWVar42 - 1);
                } while (pWVar42 != (LPCWSTR)0x0);
              }
              if (local_940 != (LPCWSTR)0x0) {
                __rustc::__rust_dealloc(local_938,(longlong)local_940 * 0x18,8);
                puVar71 = local_958;
              }
              if (local_948 != (LPCWSTR *)0x0) {
                __rustc::__rust_dealloc(puVar71,local_948,1);
              }
              bVar36 = false;
LAB_14001ab56:
              if (local_838 != 0) {
                puVar71 = local_840 + 1;
                uVar41 = local_838;
                do {
                  if (puVar71[-1] != 0) {
                    __rustc::__rust_dealloc((LPVOID)*puVar71,puVar71[-1],1);
                  }
                  puVar71 = puVar71 + 3;
                  uVar41 = uVar41 - 1;
                } while (uVar41 != 0);
              }
              if (local_848 != 0) {
                __rustc::__rust_dealloc(local_840,local_848 * 0x18,8);
              }
              if (local_800 != 0) {
                puVar71 = local_808 + 1;
                uVar41 = local_800;
                do {
                  if (puVar71[-1] != 0) {
                    __rustc::__rust_dealloc((LPVOID)*puVar71,puVar71[-1],1);
                  }
                  puVar71 = puVar71 + 3;
                  uVar41 = uVar41 - 1;
                } while (uVar41 != 0);
              }
              if (local_810 != 0) {
                __rustc::__rust_dealloc(local_808,local_810 * 0x18,8);
              }
              if (local_2a8 != (LPCWSTR)0x0) {
                puVar71 = local_2b0 + 1;
                pWVar42 = local_2a8;
                do {
                  if (puVar71[-1] != 0) {
                    __rustc::__rust_dealloc((LPVOID)*puVar71,puVar71[-1],1);
                  }
                  puVar71 = puVar71 + 3;
                  pWVar42 = (LPCWSTR)((longlong)pWVar42 + -1);
                } while (pWVar42 != (LPCWSTR)0x0);
              }
              if (local_2b8 != (LPCWSTR)0x0) {
                __rustc::__rust_dealloc(local_2b0,(longlong)local_2b8 * 0x18,8);
              }
              if (local_260 != 0) {
                puVar71 = local_268 + 1;
                lVar87 = local_260;
                do {
                  if (puVar71[-1] != 0) {
                    __rustc::__rust_dealloc((LPVOID)*puVar71,puVar71[-1],1);
                  }
                  puVar71 = puVar71 + 3;
                  lVar87 = lVar87 + -1;
                } while (lVar87 != 0);
              }
              if (local_270 != (LPCWSTR)0x0) {
                __rustc::__rust_dealloc(local_268,(longlong)local_270 * 0x18,8);
              }
              if (local_280 != 0) {
                puVar58 = local_288 + 1;
                lVar87 = local_280;
                do {
                  if (puVar58[-1] != 0) {
                    __rustc::__rust_dealloc((LPVOID)*puVar58,puVar58[-1],1);
                  }
                  puVar58 = puVar58 + 3;
                  lVar87 = lVar87 + -1;
                } while (lVar87 != 0);
              }
              if (local_290 != 0) {
                __rustc::__rust_dealloc(local_288,local_290 * 0x18,8);
              }
              if (local_2c8 != 0) {
                puVar58 = local_2d0 + 1;
                uVar41 = local_2c8;
                do {
                  if (puVar58[-1] != 0) {
                    __rustc::__rust_dealloc((LPVOID)*puVar58,puVar58[-1],1);
                  }
                  puVar58 = puVar58 + 3;
                  uVar41 = uVar41 - 1;
                } while (uVar41 != 0);
              }
              if (local_2d8 != 0) {
                __rustc::__rust_dealloc(local_2d0,local_2d8 * 0x18,8);
              }
              if (bVar36) {
                if (local_180 != (LPCWSTR)0x0) {
                  __rustc::__rust_dealloc(puStack_178,local_180,1);
                }
                if (pWVar49 != (LPCWSTR)0x0) {
                  __rustc::__rust_dealloc(local_2c0,pWVar49,1);
                }
                if (local_200 != (code *)0x0) {
                  __rustc::__rust_dealloc(local_1f8,local_200,1);
                }
              }
              if (local_220 != (LPCWSTR)0x0) {
                __rustc::__rust_dealloc(ppWStack_218,local_220,1);
              }
              if (local_1c0 != (undefined *)0x0) {
                __rustc::__rust_dealloc(local_1b8,local_1c0,1);
              }
              if (ppWVar51 != (LPCWSTR *)0x0) {
                __rustc::__rust_dealloc(local_230,ppWVar51,1);
              }
              if (local_168 != 0) {
                __rustc::__rust_dealloc(local_160,local_168,1);
              }
              goto LAB_140015a25;
            }
            <std::path::Path>::_join
                      ((undefined8 *)local_7d8,local_928,(size_t)local_910,
                       (undefined8 *******)&DAT_14015ac87,(undefined1 *)0xb);
            local_568._0_1_ = '\0';
            local_568._1_1_ = 0;
            local_568._2_2_ = 0;
            uStack_560 = 0;
            uStack_55e = 0;
            WStack_558 = 7;
            uStack_556 = 0;
            uStack_554 = 0;
            uStack_550._0_1_ = 0;
            uStack_550._2_1_ = 0;
            local_548 = 0;
            uStack_550._1_1_ = 1;
            uStack_550._4_4_ = 1;
            uStack_550._3_1_ = 1;
            pppppppuVar30 = pppppppuStack_7d0;
            pppppppuVar74 = pppppppuStack_7d0;
            lVar87 = <std::fs::OpenOptions>::_open
                               ((int *)&local_568,(longlong)pppppppuStack_7d0,(longlong)local_7c8);
            if (local_7d8 != (undefined1  [8])0x0) {
              __rustc::__rust_dealloc(pppppppuVar30,local_7d8,1);
            }
            if (lVar87 == 1) {
              start::_closure_10_(&local_568,(longlong)local_928,local_910,(ulonglong)pppppppuVar74)
              ;
              *param_1 = CONCAT26(local_568._6_2_,
                                  CONCAT24(local_568._4_2_,
                                           CONCAT22(local_568._2_2_,
                                                    CONCAT11(local_568._1_1_,(char)local_568))));
              param_1[1] = CONCAT62(uStack_55e,uStack_560);
              param_1[2] = CONCAT44(uStack_554,CONCAT22(uStack_556,WStack_558));
              CloseHandle(pppppppuVar73);
              goto LAB_14001aacf;
            }
            <std::path::Path>::_join
                      ((undefined8 *)local_7d8,local_830,local_2f8,
                       (undefined8 *******)&DAT_14015a453,&DAT_00000010);
            <std::process::Command>::new::<std::path::PathBuf>
                      (&local_568,(longlong *)local_7d8);
            ppWVar51 = local_938;
            if (local_930 != (LPCWSTR)0x0) {
              lVar77 = (longlong)local_930 * 0x18;
              lVar87 = 0;
              do {
                <std::sys::process::windows::Command>::arg
                          ((longlong)&local_568,*(void **)((longlong)ppWVar51 + lVar87 + 8),
                           *(size_t *)((longlong)ppWVar51 + lVar87 + 0x10));
                lVar87 = lVar87 + 0x18;
              } while (lVar77 != lVar87);
            }
            <std::sys::process::windows::Command>::cwd
                      ((longlong)&local_568,local_818,local_820);
            local_7d8._0_4_ = 5;
            pppppppuStack_7d0 = pppppppuVar74;
            <std::sys::process::windows::Command>::stdout
                      ((longlong)&local_568,(undefined8 *)local_7d8);
            local_7d8._0_4_ = 5;
            pppppppuStack_7d0 = pppppppuVar73;
            <std::sys::process::windows::Command>::stderr
                      ((longlong)&local_568,(undefined8 *)local_7d8);
            <std::process::Command>::spawn((longlong *)local_98,&local_568);
            if (local_98[0] == 2) {
              start::_closure_12_((ulonglong *)local_7d8,local_928,local_910,local_90);
              local_850[2] = (ulonglong)local_7c8;
              *local_850 = (ulonglong)local_7d8;
              local_850[1] = (ulonglong)pppppppuStack_7d0;
            }
            else {
              core::ptr::drop_glue::<std::process::Child>
                        ((longlong *)local_98);
              *local_850 = 0xffffffffffffffff;
            }
            param_1 = local_850;
            ppWVar51 = local_968;
            core::ptr::drop_glue::<std::process::Command>
                      (&local_568);
            hMutex = local_828;
            core::ptr::drop_glue::
            <alloc::vec::Vec<alloc::string::String>>
                      ((longlong *)&local_940);
            if (local_948 != (LPCWSTR *)0x0) {
              __rustc::__rust_dealloc(local_958,local_948,1);
            }
            core::ptr::drop_glue::
            <alloc::vec::Vec<alloc::string::String>>
                      ((longlong *)&local_848);
            core::ptr::drop_glue::
            <alloc::vec::Vec<alloc::string::String>>
                      ((longlong *)&local_810);
            core::ptr::drop_glue::
            <alloc::vec::Vec<alloc::string::String>>
                      ((longlong *)&local_2b8);
            core::ptr::drop_glue::
            <alloc::vec::Vec<alloc::string::String>>
                      ((longlong *)&local_270);
            core::ptr::drop_glue::
            <alloc::vec::Vec<alloc::string::String>>
                      ((longlong *)&local_290);
            core::ptr::drop_glue::
            <alloc::vec::Vec<alloc::string::String>>
                      ((longlong *)&local_2d8);
            if (local_220 != (LPCWSTR)0x0) {
              __rustc::__rust_dealloc(local_128,local_220,1);
            }
            if (local_1c0 != (undefined *)0x0) {
              __rustc::__rust_dealloc(local_830,local_1c0,1);
            }
            if (ppWVar51 != (LPCWSTR *)0x0) {
              __rustc::__rust_dealloc(local_230,ppWVar51,1);
            }
            if (local_168 != 0) {
              __rustc::__rust_dealloc(local_920,local_168,1);
            }
            if (local_1a0 != 0) {
              __rustc::__rust_dealloc(local_198,local_1a0,1);
            }
            if (local_258 != 0) {
              __rustc::__rust_dealloc(local_928,local_258,1);
            }
            if (local_148 != 0) {
              __rustc::__rust_dealloc(local_140,local_148,1);
            }
            lVar77 = local_960;
            if (local_1e0 != 0) {
              __rustc::__rust_dealloc(local_2f0,local_1e0,1);
            }
            goto joined_r0x00014001b28d;
          }
        }
LAB_14001bd01:
        alloc::raw_vec::handle_error(1,uVar79);
        goto LAB_14001be41;
      }
    }
  }
LAB_140017d42:
  alloc::raw_vec::handle_error(lVar77,puVar71);
LAB_14001be41:
                    /* WARNING: Does not return */
  pcVar47 = (code *)invalidInstructionException();
  (*pcVar47)();
}


