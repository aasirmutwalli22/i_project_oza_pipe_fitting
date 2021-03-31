#define ds1307_address 0x68
byte dec2bcd(byte dec) {
  return ( (dec / 10 * 16) + (dec % 10) );
}

byte bcd2dec(byte bcd) {
  return ( (bcd / 16 * 10) + (bcd % 16) );
}
struct rtcData {
  byte second;
  byte minute;
  byte hour;
  byte week_day;//sun:0, mon:1, tue, wed, thu, fri, sat: 6
  byte date;
  byte month;
  byte year;//2021: 21 //last 2 numbers
};
struct {
  void set(rtcData& _data){
    Wire.beginTransmission(ds1307_address);
    Wire.write(0);
    Wire.write(dec2bcd(_data.second));
    Wire.write(dec2bcd(_data.minute));
    Wire.write(dec2bcd(_data.hour));
    Wire.write(dec2bcd(_data.week_day));
    Wire.write(dec2bcd(_data.date));
    Wire.write(dec2bcd(_data.month));
    Wire.write(dec2bcd(_data.year));
    Wire.endTransmission();
  }
  void read(rtcData& _data){
    Wire.beginTransmission(ds1307_address);
    Wire.write(0);
    Wire.endTransmission();
    Wire.requestFrom(ds1307_address, 7);
    _data.second  = bcd2dec(Wire.read() & 0x7f);
    _data.minute  = bcd2dec(Wire.read());
    _data.hour    = bcd2dec(Wire.read() & 0x3f);
    _data.week_day = bcd2dec(Wire.read());
    _data.date    = bcd2dec(Wire.read());
    _data.month   = bcd2dec(Wire.read());
    _data.year    = bcd2dec(Wire.read());
  }
  String readString(rtcData& _data){
    read(_data);
    String _date    = _data.date    < 10 ? ("0" + String(_data.date))   : String(_data.date);
    String _month   = _data.month   < 10 ? ("0" + String(_data.month))  : String(_data.month);
    String _year    = _data.year    < 10 ? ("200" + String(_data.year)) : "20" + String(_data.year);
    String _hour    = _data.hour    < 10 ? ("0" + String(_data.hour))   : String(_data.hour);
    String _minute  = _data.minute  < 10 ? ("0" + String(_data.minute)) : String(_data.minute);
    String _second  = _data.second  < 10 ? ("0" + String(_data.second)) : String(_data.second);
    return _date + "/" + _month + "/" + _year + " " + _hour + ":" + _minute + ":" + _second;
  }
} RTC;
