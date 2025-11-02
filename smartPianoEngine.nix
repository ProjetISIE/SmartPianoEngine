{
  alsa-lib,
  cmake,
  doctest,
  libpulseaudio,
  mkDerivation,
  ninja,
  qtmultimedia,
  rtmidi,
  self,
}:
mkDerivation {
  pname = "smart-piano-engine";
  version = "0.1.0";
  src = self;
  nativeBuildInputs = [
    cmake # Modern build tool
    doctest # Testing framework
    ninja # Modern build tool
    # pkg-config # Build tool
    # wrapQtAppsHook # Qt build tool, included by qt5.mkDerivation
  ];
  buildInputs = [
    alsa-lib # Audio lib
    libpulseaudio # Audio lib
    # qtbase # Qt itself, included by qt5.mkDerivation
    qtmultimedia # Qt Multimedia lib
    rtmidi # Lib
  ];
}
