# ProjectRadarSequencer

A audiovisual installation that uses a pair of SICK laser scanners to react to peoples proximity.

## Configuration

```
git clone git@github.com:openframeworks/openFrameworks.git
cd openFrameworks
git checkout develop

cd addons/
git clone git@github.com:rezaali/ofxUI.git
git clone git@github.com:ZigelbaumCoelho/ofxSick.git
git clone git@github.com:danomatika/ofxMidi.git
git clone git@github.com:kylemcdonald/ofxDmx.git
git clone git@github.com:kylemcdonald/ofxCv.git

cd ../apps/
git clone git@github.com:HellicarAndLewis/ProjectRadarSequencer.git
```

There is one small change you need to make to ofxUI. Line 219 of `ofxUI.h` should be changed to:

```c++
#define OFX_UI_FONT_NAME "sans-serif"
```