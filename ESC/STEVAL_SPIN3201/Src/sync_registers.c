
/**
  ******************************************************************************
  * @file    sync_registers.c
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file provides firmware functions that implement the register
  * access for the synchronous part of the MCP protocol.
  *
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include "mc_type.h"
#include "string.h"
#include "register_interface.h"
#include "mc_config.h"
#include "mcp.h"
#include "mcp_config.h"
#include "mc_configuration_registers.h"

uint8_t RI_SetRegisterGlobal(uint16_t regID, uint8_t typeID, uint8_t *data, uint16_t *size, int16_t dataAvailable)
{
  uint8_t retVal = MCP_CMD_OK;
  switch(typeID)
  {
    case TYPE_DATA_8BIT:
    {
      switch (regID)
      {
        case MC_REG_STATUS:
        {
          retVal = MCP_ERROR_RO_REG;
          break;
        }

        default:
        {
          retVal = MCP_ERROR_UNKNOWN_REG;
          break;
        }
      }
      *size = 1;
      break;
    }

    case TYPE_DATA_16BIT:
    {
      switch (regID)
      {

        case MC_REG_BUS_VOLTAGE:
        case MC_REG_HEATS_TEMP:
        case MC_REG_MOTOR_POWER:
        {
          retVal = MCP_ERROR_RO_REG;
          break;
        }

        case MC_REG_DAC_USER1:
        case MC_REG_DAC_USER2:
        break;

        default:
        {
          retVal = MCP_ERROR_UNKNOWN_REG;
          break;
        }
      }
      *size = 2;
      break;
    }

    case TYPE_DATA_32BIT:
    {

      switch (regID)
      {
        case MC_REG_FAULTS_FLAGS:
        {
          retVal = MCP_ERROR_RO_REG;
          break;
        }

        default:
        {
          retVal = MCP_ERROR_UNKNOWN_REG;
          break;
        }
      }
      *size = 4;
      break;
    }

    case TYPE_DATA_RAW:
    {
      uint16_t rawSize = *(uint16_t *)data; //cstat !MISRAC2012-Rule-11.3
      /* The size consumed by the structure is the structure size + 2 bytes used to store the size */
      *size = rawSize + 2U;
      uint8_t *rawData = data; /* rawData points to the first data (after size extraction) */
      rawData++;
      rawData++;

      if (*size > (uint16_t)dataAvailable)
      {
        /* The decoded size of the raw structure can not match with transmitted buffer, error in buffer
           construction */
        *size = 0;
        retVal = MCP_ERROR_BAD_RAW_FORMAT; /* This error stop the parsing of the CMD buffer */
      }
      else
      {
        switch (regID)
        {
          case MC_REG_APPLICATION_CONFIG:
          case MC_REG_GLOBAL_CONFIG:
          case MC_REG_FOCFW_CONFIG:
          {
            retVal = MCP_ERROR_RO_REG;
            break;
          }

          default:
          {
            retVal = MCP_ERROR_UNKNOWN_REG;
            break;
          }
        }

      }
      break;
    }

    default:
    {
      retVal = MCP_ERROR_BAD_DATA_TYPE;
      *size =0; /* From this point we are not able anymore to decode the RX buffer */
      break;
    }
  }
  return (retVal);
}

uint8_t RI_SetRegisterMotor1(uint16_t regID, uint8_t typeID, uint8_t *data, uint16_t *size, int16_t dataAvailable)
{
  uint8_t retVal = MCP_CMD_OK;
  uint8_t motorID=0;
  MCI_Handle_t *pMCIN = &Mci[motorID];

  switch(typeID)
  {
    case TYPE_DATA_8BIT:
    {
      switch (regID)
      {
        case MC_REG_STATUS:
          {
            retVal = MCP_ERROR_RO_REG;
            break;
          }
        case MC_REG_CONTROL_MODE:
        {
          uint8_t regdata8 = *data;

          if ((uint8_t)MCM_TORQUE_MODE == regdata8)
          {

            MCI_ExecTorqueRamp(pMCIN, MCI_GetTeref(pMCIN), 0);
          }
          else
          {
            /* Nothing to do */
          }

          if ((uint8_t)MCM_SPEED_MODE == regdata8)
          {
            MCI_ExecSpeedRamp(pMCIN, MCI_GetMecSpeedRefUnit(pMCIN), 0);
          }
          else
          {
            /* Nothing to do */
          }
          break;
        }

        case MC_REG_RUC_STAGE_NBR:
        {
          retVal = MCP_ERROR_RO_REG;
          break;
        }

        default:
        {
          retVal = MCP_ERROR_UNKNOWN_REG;
          break;
        }
      }
      *size = 1;
      break;
    }

    case TYPE_DATA_16BIT:
    {
      uint16_t regdata16 = *(uint16_t *)data; //cstat !MISRAC2012-Rule-11.3
      switch (regID)
      {

        case MC_REG_SPEED_KP:
        {
          PID_SetKP(&PIDSpeedHandle_M1, (int16_t)regdata16);
          break;
        }

        case MC_REG_SPEED_KI:
        {
          PID_SetKI(&PIDSpeedHandle_M1, (int16_t)regdata16);
          break;
        }

        case MC_REG_SPEED_KD:
        {
          PID_SetKD(&PIDSpeedHandle_M1, (int16_t)regdata16);
          break;
        }

        case MC_REG_BUS_VOLTAGE:
        case MC_REG_HEATS_TEMP:
        case MC_REG_MOTOR_POWER:
        {
          retVal = MCP_ERROR_RO_REG;
          break;
        }

        case MC_REG_FLUXWK_BUS_MEAS:
        {
          retVal = MCP_ERROR_RO_REG;
          break;
        }

        case MC_REG_V_Q:
        case MC_REG_V_D:
        case MC_REG_V_ALPHA:
        case MC_REG_V_BETA:
        {
          retVal = MCP_ERROR_RO_REG;
          break;
        }

        case MC_REG_STOPLL_C1:
        {
          int16_t hC1;
          int16_t hC2;
          STO_PLL_GetObserverGains(&STO_PLL_M1, &hC1, &hC2);
          STO_PLL_SetObserverGains(&STO_PLL_M1, (int16_t)regdata16, hC2);
          break;
        }

        case MC_REG_STOPLL_C2:
        {
          int16_t hC1;
          int16_t hC2;
          STO_PLL_GetObserverGains(&STO_PLL_M1, &hC1, &hC2);
          STO_PLL_SetObserverGains(&STO_PLL_M1, hC1, (int16_t)regdata16);
          break;
        }

        case MC_REG_STOPLL_KI:
        {
          PID_SetKI (&(&STO_PLL_M1)->PIRegulator, (int16_t)regdata16);
          break;
        }

        case MC_REG_STOPLL_KP:
        {
          PID_SetKP (&(&STO_PLL_M1)->PIRegulator, (int16_t)regdata16);
          break;
        }

        case MC_REG_STOPLL_EL_ANGLE:
        case MC_REG_STOPLL_ROT_SPEED:
        case MC_REG_STOPLL_BEMF_ALPHA:
        case MC_REG_STOPLL_BEMF_BETA:
        {
          retVal = MCP_ERROR_RO_REG;
          break;
        }

        case MC_REG_DAC_USER1:
        case MC_REG_DAC_USER2:
          break;

        default:
        {
          retVal = MCP_ERROR_UNKNOWN_REG;
          break;
        }
      }
      *size = 2;
      break;
    }

    case TYPE_DATA_32BIT:
    {
      uint32_t regdata32 = (((uint32_t)(*(uint16_t *)&data[2])) << 16) | *(uint16_t *)data;

      switch (regID)
      {
        case MC_REG_FAULTS_FLAGS:
        case MC_REG_SPEED_MEAS:
        {
          retVal = MCP_ERROR_RO_REG;
          break;
        }

        case MC_REG_SPEED_REF:
        {
          MCI_ExecSpeedRamp(pMCIN,((((int16_t)regdata32) * ((int16_t)SPEED_UNIT)) / (int16_t)U_RPM), 0);
          break;
        }

        case MC_REG_STOPLL_EST_BEMF:
        case MC_REG_STOPLL_OBS_BEMF:
        {
          retVal = MCP_ERROR_RO_REG;
          break;
        }

        default:
        {
          retVal = MCP_ERROR_UNKNOWN_REG;
          break;
        }
      }
      *size = 4;
      break;
    }

    case TYPE_DATA_RAW:
    {
      uint16_t rawSize = *(uint16_t *)data; //cstat !MISRAC2012-Rule-11.3
      /* The size consumed by the structure is the structure size + 2 bytes used to store the size */
      *size = rawSize + 2U;
      uint8_t *rawData = data; /* rawData points to the first data (after size extraction) */
      rawData++;
      rawData++;

      if (*size > (uint16_t)dataAvailable)
      {
        /* The decoded size of the raw structure can not match with transmitted buffer, error in buffer
           construction */
        *size = 0;
        retVal = MCP_ERROR_BAD_RAW_FORMAT; /* This error stop the parsing of the CMD buffer */
      }
      else
      {
        switch (regID)
        {
          case MC_REG_APPLICATION_CONFIG:
          case MC_REG_GLOBAL_CONFIG:
          case MC_REG_FOCFW_CONFIG:
          {
            retVal = MCP_ERROR_RO_REG;
            break;
          }

          case MC_REG_SPEED_RAMP:
          {
            int32_t rpm;
            uint16_t duration;

            /* 32 bits access are splited into 2x16 bits access */
            rpm = (((int32_t)(*(int16_t *)&rawData[2])) << 16) | *(uint16_t *)rawData; //cstat !MISRAC2012-Rule-11.3
            duration = *(uint16_t *)&rawData[4]; //cstat !MISRAC2012-Rule-11.3
            MCI_ExecSpeedRamp(pMCIN, (int16_t)((rpm * SPEED_UNIT) / U_RPM), duration);
            break;
          }

          default:
          {
            retVal = MCP_ERROR_UNKNOWN_REG;
            break;
          }
        }
      }
      break;
    }

    default:
    {
      retVal = MCP_ERROR_BAD_DATA_TYPE;
      *size =0; /* From this point we are not able anymore to decode the RX buffer */
      break;
    }
  }
  return (retVal);
}

uint8_t RI_GetRegisterGlobal(uint16_t regID,uint8_t typeID,uint8_t * data,uint16_t *size,int16_t freeSpace){
    uint8_t retVal = MCP_CMD_OK;
    switch (typeID)
    {
      case TYPE_DATA_8BIT:
      {
        if (freeSpace > 0)
        {
          switch (regID)
          {
            default:
            {
              retVal = MCP_ERROR_UNKNOWN_REG;
              break;
            }
          }
          *size = 1;
        }
        else
        {
          retVal = MCP_ERROR_NO_TXSYNC_SPACE;
        }
        break;
      }

      case TYPE_DATA_16BIT:
      {
        if (freeSpace >= 2)
        {
          switch (regID)
          {
            case MC_REG_DAC_USER1:
            case MC_REG_DAC_USER2:
              break;

            default:
            {
              retVal = MCP_ERROR_UNKNOWN_REG;
              break;
            }
          }
          *size = 2;
        }
        else
        {
          retVal = MCP_ERROR_NO_TXSYNC_SPACE;
        }
        break;
      }

      case TYPE_DATA_32BIT:
      {
        if (freeSpace >= 4)
        {
          switch (regID)
          {

            default:
            {
              retVal = MCP_ERROR_UNKNOWN_REG;
              break;
            }
          }
          *size = 4;
        }
        else
        {
          retVal = MCP_ERROR_NO_TXSYNC_SPACE;
        }
        break;
      }

      case TYPE_DATA_STRING:
      {
        switch (regID)
        {
          default:
          {

            retVal = MCP_ERROR_UNKNOWN_REG;
            *size= 0 ; /* */

            break;
          }
        }
        break;

      }
      case TYPE_DATA_RAW:
      {
        /* First 2 bytes of the answer is reserved to the size */
        uint16_t *rawSize = (uint16_t *)data; //cstat !MISRAC2012-Rule-11.3
        uint8_t * rawData = data;
        rawData++;
        rawData++;

        switch (regID)
        {
          case MC_REG_GLOBAL_CONFIG:
          {
            *rawSize = (uint16_t)sizeof(GlobalConfig_reg_t);
            if (((*rawSize) + 2U) > (uint16_t)freeSpace)
            {
              retVal = MCP_ERROR_NO_TXSYNC_SPACE;
            }
            else
            {
              (void)memcpy(rawData, &globalConfig_reg, sizeof(GlobalConfig_reg_t));
            }
            break;
          }
          case MC_REG_ASYNC_UARTA:
          case MC_REG_ASYNC_UARTB:
          case MC_REG_ASYNC_STLNK:
          default:
          {
            retVal = MCP_ERROR_UNKNOWN_REG;
            break;
          }
        }

        /* Size of the answer is size of the data + 2 bytes containing data size */
        *size = (*rawSize) + 2U;
        break;
      }

      default:
      {
        retVal = MCP_ERROR_BAD_DATA_TYPE;
        break;
      }
    }
  return (retVal);
}

  uint8_t RI_GetRegisterMotor1(uint16_t regID,uint8_t typeID,uint8_t * data,uint16_t *size,int16_t freeSpace) {
    uint8_t retVal = MCP_CMD_OK;
    uint8_t motorID=0;
    MCI_Handle_t *pMCIN = &Mci[motorID];
    BusVoltageSensor_Handle_t* BusVoltageSensor= &BusVoltageSensor_M1._Super;
    switch (typeID)
    {
      case TYPE_DATA_8BIT:
      {
        if (freeSpace > 0)
        {
          switch (regID)
          {
            case MC_REG_STATUS:
            {
              *data = (uint8_t)MCI_GetSTMState(pMCIN);
              break;
            }

            case MC_REG_CONTROL_MODE:
            {
              *data = (uint8_t)MCI_GetControlMode(pMCIN);
              break;
            }

            case MC_REG_RUC_STAGE_NBR:
            {
              *data = (uint8_t)RUC_GetNumberOfPhases(&RevUpControlM1);
              break;
            }

            default:
            {
              retVal = MCP_ERROR_UNKNOWN_REG;
              break;
            }
          }
          *size = 1;
        }
        else
        {
          retVal = MCP_ERROR_NO_TXSYNC_SPACE;
        }
        break;
      }

      case TYPE_DATA_16BIT:
      {
        uint16_t *regdataU16 = (uint16_t *)data; //cstat !MISRAC2012-Rule-11.3
        int16_t *regdata16 = (int16_t *) data; //cstat !MISRAC2012-Rule-11.3

        if (freeSpace >= 2)
        {
          switch (regID)
          {

            case MC_REG_SPEED_KP:
            {
              *regdata16 = PID_GetKP(&PIDSpeedHandle_M1);
              break;
            }

            case MC_REG_SPEED_KI:
            {
              *regdata16 = PID_GetKI(&PIDSpeedHandle_M1);
              break;
            }

            case MC_REG_SPEED_KD:
            {
              *regdata16 = PID_GetKD(&PIDSpeedHandle_M1);
              break;
            }

            case MC_REG_BUS_VOLTAGE:
            {
              *regdataU16 = VBS_GetAvBusVoltage_V(BusVoltageSensor);
              break;
            }

            case MC_REG_HEATS_TEMP:
            {
              *regdata16 = NTC_GetAvTemp_C(&TempSensor_M1);
              break;
            }

            case MC_REG_V_Q:
            {
              *regdata16 = MCI_GetVqd(pMCIN).q;
              break;
            }

            case MC_REG_V_D:
            {
              *regdata16 = MCI_GetVqd(pMCIN).d;
              break;
            }

            case MC_REG_V_ALPHA:
            {
              *regdata16 = MCI_GetValphabeta(pMCIN).alpha;
              break;
            }

            case MC_REG_V_BETA:
            {
              *regdata16 = MCI_GetValphabeta(pMCIN).beta;
              break;
            }

            case MC_REG_STOPLL_EL_ANGLE:
            {
              //cstat !MISRAC2012-Rule-11.3
              *regdata16 = SPD_GetElAngle((SpeednPosFdbk_Handle_t *)&STO_PLL_M1);
              break;
            }

            case MC_REG_STOPLL_ROT_SPEED:
            {
              //cstat !MISRAC2012-Rule-11.3
              *regdata16 = SPD_GetS16Speed((SpeednPosFdbk_Handle_t *)&STO_PLL_M1);
              break;
            }

            case MC_REG_STOPLL_BEMF_ALPHA:
            {
              *regdata16 = STO_PLL_GetEstimatedBemf(&STO_PLL_M1).alpha;
              break;
            }

            case MC_REG_STOPLL_BEMF_BETA:
            {
              *regdata16 = STO_PLL_GetEstimatedBemf(&STO_PLL_M1).beta;
              break;
            }

            case MC_REG_STOPLL_C1:
            {
              int16_t hC1;
              int16_t hC2;
              STO_PLL_GetObserverGains(&STO_PLL_M1, &hC1, &hC2);
              *regdata16 = hC1;
              break;
            }

            case MC_REG_STOPLL_C2:
            {
              int16_t hC1;
              int16_t hC2;
              STO_PLL_GetObserverGains(&STO_PLL_M1, &hC1, &hC2);
              *regdata16 = hC2;
              break;
            }

            case MC_REG_STOPLL_KI:
            {
              *regdata16 = PID_GetKI (&(&STO_PLL_M1)->PIRegulator);
              break;
            }

            case MC_REG_STOPLL_KP:
            {
              *regdata16 = PID_GetKP (&(&STO_PLL_M1)->PIRegulator);
              break;
            }

            case MC_REG_DAC_USER1:
            case MC_REG_DAC_USER2:
              break;

            default:
            {
              retVal = MCP_ERROR_UNKNOWN_REG;
              break;
            }
          }
          *size = 2;
        }
        else
        {
          retVal = MCP_ERROR_NO_TXSYNC_SPACE;
        }
        break;
      }

      case TYPE_DATA_32BIT:
      {
        uint32_t *regdataU32 = (uint32_t *)data; //cstat !MISRAC2012-Rule-11.3
        int32_t *regdata32 = (int32_t *)data; //cstat !MISRAC2012-Rule-11.3

        if (freeSpace >= 4)
        {
          switch (regID)
          {
            case MC_REG_FAULTS_FLAGS:
            {
              *regdataU32 = MCI_GetFaultState(pMCIN);
              break;
            }
            case MC_REG_SPEED_MEAS:
            {
              *regdata32 = (((int32_t)MCI_GetAvrgMecSpeedUnit(pMCIN) * U_RPM) / SPEED_UNIT);
              break;
            }

            case MC_REG_SPEED_REF:
            {
              *regdata32 = (((int32_t)MCI_GetMecSpeedRefUnit(pMCIN) * U_RPM) / SPEED_UNIT);
              break;
            }

            case MC_REG_STOPLL_EST_BEMF:
            {
              *regdata32 = STO_PLL_GetEstimatedBemfLevel(&STO_PLL_M1);
              break;
            }

            case MC_REG_STOPLL_OBS_BEMF:
            {
              *regdata32 = STO_PLL_GetObservedBemfLevel(&STO_PLL_M1);
              break;
            }

            case MC_REG_MOTOR_POWER:
            {
              FloatToU32 ReadVal; //cstat !MISRAC2012-Rule-19.2
              ReadVal.Float_Val = PQD_GetAvrgElMotorPowerW(pMPM[M1]);
              *regdataU32 = ReadVal.U32_Val; //cstat !UNION-type-punning
              break;
            }

            default:
            {
              retVal = MCP_ERROR_UNKNOWN_REG;
              break;
            }
          }
          *size = 4;
        }
        else
        {
          retVal = MCP_ERROR_NO_TXSYNC_SPACE;
        }
        break;
      }

      case TYPE_DATA_STRING:
      {
        switch (regID)
        {

          default:
          {
            retVal = MCP_ERROR_UNKNOWN_REG;
            *size= 0 ; /* */
            break;
          }
        }
        break;
      }

      case TYPE_DATA_RAW:
      {
        /* First 2 bytes of the answer is reserved to the size */
        uint16_t *rawSize = (uint16_t *)data; //cstat !MISRAC2012-Rule-11.3
        uint8_t * rawData = data;
        rawData++;
        rawData++;

        switch (regID)
        {
          case MC_REG_APPLICATION_CONFIG:
          {
            *rawSize = (uint16_t)sizeof(ApplicationConfig_reg_t);
            if (((*rawSize) + 2U) > (uint16_t)freeSpace)
            {
              retVal = MCP_ERROR_NO_TXSYNC_SPACE;
            }
            else
            {
              ApplicationConfig_reg_t const *pApplicationConfig_reg = ApplicationConfig_reg[motorID];
              (void)memcpy(rawData, (const uint8_t *)pApplicationConfig_reg, sizeof(ApplicationConfig_reg_t));
            }
            break;
          }

          case MC_REG_FOCFW_CONFIG:
          {
            *rawSize = (uint16_t)sizeof(FOCFwConfig_reg_t);
            if (((*rawSize) + 2U) > (uint16_t)freeSpace)
            {
              retVal = MCP_ERROR_NO_TXSYNC_SPACE;
            }
            else
            {
              FOCFwConfig_reg_t const *pFOCConfig_reg = FOCConfig_reg[motorID];
              (void)memcpy(rawData, (const uint8_t *)pFOCConfig_reg, sizeof(FOCFwConfig_reg_t));
            }

            break;
          }
          case MC_REG_SCALE_CONFIG:
          {
            *rawSize = 12;
            if ((*rawSize) +2U > (uint16_t)freeSpace)
            {
              retVal = MCP_ERROR_NO_TXSYNC_SPACE;
            }
            else
            {
              memcpy(rawData, &scaleParams_M1, sizeof(ScaleParams_t) );
            }
            break;
          }
          case MC_REG_SPEED_RAMP:
          {
            uint16_t *rpm16p = (uint16_t *)rawData; //cstat !MISRAC2012-Rule-11.3
            uint16_t *duration = (uint16_t *)&rawData[4]; //cstat !MISRAC2012-Rule-11.3
            int32_t rpm32 = ((int32_t)(MCI_GetLastRampFinalSpeed(pMCIN) * U_RPM) / (int32_t)SPEED_UNIT);
            *rpm16p = (uint16_t)rpm32;
            *(rpm16p+1) = (uint16_t)(rpm32>>16);
            *duration = MCI_GetLastRampFinalDuration(pMCIN);
            *rawSize = 6;
            break;
          }

          case MC_REG_ASYNC_UARTA:
          case MC_REG_ASYNC_UARTB:
          case MC_REG_ASYNC_STLNK:
          default:
          {
            retVal = MCP_ERROR_UNKNOWN_REG;
            break;
          }
        }

        /* Size of the answer is size of the data + 2 bytes containing data size */
        *size = (*rawSize) + 2U;
        break;
      }

      default:
      {
        retVal = MCP_ERROR_BAD_DATA_TYPE;
        break;
      }
    }
    return (retVal);
  }

/************************ (C) COPYRIGHT 2025 STMicroelectronics *****END OF FILE****/
