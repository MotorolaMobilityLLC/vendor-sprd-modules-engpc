#ifndef TSX_RAWDATA_H_
#define TSX_RAWDATA_H_

typedef unsigned short uint16;
typedef unsigned int uint32;

/*
Type = 98;
subtype = 0;

typedef enum
{
    ¡­
    DIAG_AP_CMD_SET_PMIC_AFC_CDAC   = 0x0025,
    DIAG_AP_CMD_PMIC_AFC_AMP_CALI   = 0x0026
    DIAG_AP_CMD_LOAD_PMIC_AFC_DATA  = 0x0027,
    DIAG_AP_CMD_SAVE_PMIC_AFC_DATA  = 0x0028,
    DIAG_AP_CMD_SET_PMIC_AFC_MODE	  = 0x0029,
} DIAG_AP_CMD_E;

typedef struct 
{
    uint16  cmd;        // DIAG_AP_CMD_E
    uint16  length;     // Length of request data
} TOOLS_DIAG_AP_REQ_T;

typedef struct 
{
    uint16 status;      //status of cmd 0:succ, != 0: fail
    uint16 length;      //length of confirm data
} TOOLS_DIAG_AP_CNF_T;

Req: TOOLS_DIAG_AP_REQ_T + data
Conf: TOOLS_DIAG_AP_CNF_T + data
*/


//cmd = DIAG_AP_CMD_SET_PMIC_AFC_MODE;
//Req£ºTOOLS_DIAG_AP_REQ_T + DIAG_AP_PMIC_AFC_MODE
//Conf£ºTOOLS_DIAG_AP_CNF_T
typedef struct
{
    uint16 mode;    //0: low mode; 1:High mode
    uint16 reserved;
}DIAG_AP_PMIC_AFC_MODE;

//cmd = DIAG_AP_CMD_SET_PMIC_AFC_CDAC;
//Req£ºTOOLS_DIAG_AP_REQ_T + DIAG_AP_SET_PMIC_CDAC
//Conf£ºTOOLS_DIAG_AP_CNF_T
typedef struct
{
    uint16 CDAC;
    uint16 reserved;
}DIAG_AP_SET_PMIC_CDAC;

//cmd = DIAG_AP_CMD_PMIC_AFC_AMP_CALI;
//Req£ºTOOLS_DIAG_AP_REQ_T
//Conf£ºTOOLS_DIAG_AP_CNF_T + DIAG_AP_PMIC_AFC_AMP_DATA_T
typedef struct
{
    uint16 Amp_26M;  // HP or LP mode value specified by PCr
    uint16 reserved;
}DIAG_AP_PMIC_AFC_AMP_DATA_T;

//cmd = DIAG_AP_CMD_SAVE_PMIC_AFC_DATA;
//Req£ºTOOLS_DIAG_AP_REQ_T + DIAG_AP_PMIC_AFC_CALI_DATA_T
//Conf£ºTOOLS_DIAG_AP_CNF_T

//cmd = DIAG_AP_CMD_LOAD_PMIC_AFC_DATA;
//Req£ºTOOLS_DIAG_AP_REQ_T 
//Conf£ºTOOLS_DIAG_AP_CNF_T + DIAG_AP_PMIC_AFC_CALI_DATA_T
typedef struct
{
    uint16 CDAC;
    uint16 HP_Mode_Amp_26M;
    uint16 LP_Mode_Amp_26M;
    uint16 LP_Mode_Freq_Drift_32K;
    uint16 reserved[4];
}DIAG_AP_PMIC_AFC_CALI_DATA_T;


#endif