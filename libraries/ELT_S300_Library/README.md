# ELT SENSOR S-300 Series CO2 sensor Library for Arduino

![ELT S-300](http://www.eltsensor.co.kr/en/wp-content/uploads/sites/2/2017/11/n_s300.jpg)

This is an Arduino library for the S-300 series CO2 sensor via I2C interface.

- http://www.eltsensor.co.kr/en/portfolio/s-300/

These sensors use **I2C** to communicatge, 2pins are required to interface.

**CAUTION!**  
**This library is personally created independently of ELT, and ELT has nothing to do with defects in this library.**

**SORRY**  
**Please allow most of the comments written in the source code in the example to be in Japanese.**

## Installation

Use the Arduino Library Manager to install this library. If you're unfamiliar
with how this works, we have a great tutorial on Arduino library installation
at: http://stupiddog.jp/note/archives/266
(In Japanese)


## ToDo

 - Although this library has functions such as MCDL as methods of this library, it is not reflected in the sample program, so it is necessary to create a sample program to introduce them.

 - It is necessary to enhance the functional description for hardware reset.


-----
# ELT SENSOR S-300 シリーズ 二酸化炭素(CO2)センサー Arduino用ライブラリ

![ELT S-300](http://eltsensor.co.kr/2016/jp/images/sub/prod/oem_modules/co2-smallest-sensors-module.jpg)

このライブラリは、ArduinoとELT社製二酸化炭素センサS-300をI2Cインタフェースで接続して使用するためのものです。

- http://eltsensor.co.kr/2016/jp/products/oem_modules/S-300.html

## 注意
このライブラリは私個人が勝手に作成したものでソフトウェア不具合が生じてもELT社には一切の関係はありませんので、ご了承ください。

## インストール

Arduino IDEのライブラリ管理にアクセスして適当にインストールしてください。

## 今後の予定

 - 手動校正などの機能はメソッドとして実装されているのですが、サンプルプログラムに反映されていないのでそのプログラムも紹介できると良い。
 - ハードウェアによるリセット機能を実装したい。

