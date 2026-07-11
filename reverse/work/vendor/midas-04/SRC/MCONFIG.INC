;*      MCONFIG.INC
;*
;* MIDAS Sound System configuration. Meant to be used with the simplified
;* MIDAS API, MIDAS.C
;*
;* Copyright 1995 Petteri Kangaslampi and Jarno Paananen
;*
;* This file is part of the MIDAS Sound System, and may only be
;* used, modified and distributed under the terms of the MIDAS
;* Sound System license, LICENSE.TXT. By continuing to use,
;* modify or distribute this file you indicate that you have
;* read the license and understand and accept it fully.
;*



;/***************************************************************************\
;*
;* Function:    int midasConfig(void)
;*
;* Description: MIDAS Sound System configuration. Prompts the user for all
;*              configuration information and sets the MIDAS variables
;*              accordingly. Call before midasInit() but after
;*              midasSetDefaults().
;*
;* Returns:     1 if configuration was successful, 0 if not (Esc was pressed)
;*
;\***************************************************************************/

GLOBAL  LANG midasConfig : far




;/***************************************************************************\
;*
;* Function:    void midasLoadConfig(char *fileName);
;*
;* Description: Loads configuration from file saved using midasSaveConfig().
;*
;* Input:       char *fileName          configuration file name, ASCIIZ
;*
;\***************************************************************************/

GLOBAL  LANG midasLoadConfig : far




;/***************************************************************************\
;*
;* Function:    void midasSaveConfig(char *fileName);
;*
;* Description: Saves configuration to a file
;*
;* Input:       char *fileName          configuration file name, ASCIIZ
;*
;\***************************************************************************/

GLOBAL  LANG midasSaveConfig : far
