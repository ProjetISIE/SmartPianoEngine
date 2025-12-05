{
  alsa-lib,
  cmake,
  doctest,
  mkDerivation,
  ninja,
  miniaudio,
  qtmultimedia,
  rtmidi,
  soundfont-fluid,
  self,
}:
mkDerivation {
  pname = "engine";
  version = "0.0.0";
  src = self;
  doCheck = true; # Enable tests
  cmakeFlags = [
    "-DSOUNDFONT_PATH=${soundfont-fluid}/share/soundfonts/FluidR3_GM2-2.sf2"
  ];
  nativeBuildInputs = [
    cmake # Modern build tool
    doctest # Testing framework
    ninja # Modern build tool
    # pkg-config # Build tool
    # wrapQtAppsHook # Qt build tool, included by qt5.mkDerivation
  ];
  buildInputs = [
    alsa-lib # Audio lib
    # qtbase # Qt itself, included by qt5.mkDerivation
    # jack2 # Audio interconnection lib
    # libjack2 # Audio interconnection lib
    miniaudio # Audio lib
    qtmultimedia # Qt Multimedia lib
    rtmidi # Lib
    soundfont-fluid # SoundFont for FluidSynth
  ];
}
