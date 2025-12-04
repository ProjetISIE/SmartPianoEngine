{
  alsa-lib,
  cmake,
  doctest,
  mkDerivation,
  ninja,
  libjack2,
  miniaudio,
  pkg-config,
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
  nativeBuildInputs = [
    cmake # Modern build tool
    doctest # Testing framework
    ninja # Modern build tool
    pkg-config # Build tool
    # wrapQtAppsHook # Qt build tool, included by qt5.mkDerivation
  ];
  buildInputs = [
    alsa-lib # Audio lib
    # qtbase # Qt itself, included by qt5.mkDerivation
    # jack2 # Audio interconnection lib
    libjack2 # Audio interconnection lib
    miniaudio # Audio lib
    qtmultimedia # Qt Multimedia lib
    rtmidi # Lib
    soundfont-fluid # SoundFont for FluidSynth
  ];
}
