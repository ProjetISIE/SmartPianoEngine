{
  alsa-lib,
  libpulseaudio,
  mkDerivation,
  qtmultimedia,
  rtmidi,
  self,
}:
mkDerivation {
  pname = "smart-piano-engine";
  version = "0.1.0";
  src = self;
  nativeBuildInputs = [
    # qmake # Qt build tool, included by qt5.mkDerivation
    # qttools # Qt tooling
    # wrapQtAppsHook # Qt build tool, included by qt5.mkDerivation
  ];
  buildInputs = [
    alsa-lib # Audio lib
    libpulseaudio # Audio lib
    # qtbase # Qt itself, included by qt5.mkDerivation
    qtmultimedia # Qt Multimedia lib
    rtmidi # Lib
  ];
  configurePhase = ''
    qmake
  '';
  installPhase = ''
    install -D PianoTrainerMDJV1 $out/bin/engine
  '';
}
