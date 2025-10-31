/**********************************************************************
  ZED_F9P.ino

  Support for the u-blox ZED-F9P GNSS receiver
**********************************************************************/

#ifdef  USE_ZED_F9P
//*********************************************************************
// Store horizontal position data
// Inputs:
//   ubxDataStruct: Data structure containing high precision data
void r4aZedF9pStoreHPdata(UBX_NAV_HPPOSLLH_data_t * ubxDataStruct)
{
    zedf9p.storeHPdata(ubxDataStruct);
}

//*********************************************************************
// Store vertical position and time data
// Inputs:
//   ubxDataStruct: Data structure containing position, velocity and time
void r4aZedF9pStorePVTdata(UBX_NAV_PVT_data_t * ubxDataStruct)
{
    zedf9p.storePVTdata(ubxDataStruct);
}
#endif  // USE_ZED_F9P
