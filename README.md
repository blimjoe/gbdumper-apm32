**1. Build MCU:**  
   Open ```project.uvprojx``` in Keil 5.
   Buiding host keil5 info:
   > IDE-Version:  
   > μVision V5.38.0.0  
   > Copyright (C) 2022 ARM Ltd and ARM Germany GmbH. All rights reserved.  
   >  
   > Tool Version Numbers:  
   > Toolchain:        MDK-ARM Plus  Version: 5.38.0.0  
   > Toolchain Path:    C:\Keil_v5\ARM\ARMCLANG\Bin  
   > C Compiler:         ArmClang.exe        V6.19  
   > Assembler:          Armasm.exe        V6.19  
   > Linker/Locator:     ArmLink.exe        V6.19  
   > Library Manager:    ArmAr.exe        V6.19  
   > Hex Converter:      FromElf.exe        V6.19  
   > CPU DLL:               SARMCM3.DLL          V5.38.0.0  
   > Dialog DLL:         TCM.DLL              V1.56.4.0  
   > Target DLL:             CMSIS_AGDI.dll       V1.33.15.0  
   > Dialog DLL:         TCM.DLL              V1.56.4.0  
  
**2. Flash**  
   Using J-Flash.

**3. Building host software**  
   Host: Linux X86_64
   ```
   $ gcc -o host/dumpAll host/dumpAll.c
   ```

**4. Usage**  
   Plugin usb type C to usb type A.
   run
   ```
   $ cd host/
   $ ./dumpAll
   ```  
