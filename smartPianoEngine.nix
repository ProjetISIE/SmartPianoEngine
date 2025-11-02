{
  alsa-lib,
  cmake,
  doctest,
  mkDerivation,
  ninja,
  jack2,
  qtmultimedia,
  rtmidi,
  self,
}:
mkDerivation {
  pname = "smart-piano-engine";
  version = "0.1.0";
  src = self;
  doCheck = true; # Enable tests
  nativeBuildInputs = [
    cmake # Modern build tool
    doctest # Testing framework
    ninja # Modern build tool
    # wrapQtAppsHook # Qt build tool, included by qt5.mkDerivation
  ];
  buildInputs = [
    alsa-lib # Audio lib
    # qtbase # Qt itself, included by qt5.mkDerivation
    jack2 # Audio interconnection lib
    qtmultimedia # Qt Multimedia lib
    rtmidi # Lib
  ];
}
