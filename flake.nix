{
  description = "Nix flake C++23 development environment";
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  outputs =
    { self, nixpkgs }:
    let
      systems =
        f:
        let
          localSystems = [
            "aarch64-linux"
            "x86_64-linux"
            "aarch64-darwin"
          ];
          crossSystem = "aarch64-linux";
        in
        nixpkgs.lib.genAttrs localSystems (
          localSystem:
          f (import nixpkgs { inherit localSystem; }) (
            import nixpkgs {
              inherit localSystem;
              inherit crossSystem;
            }
          )
        );
    in
    {
      packages = systems (
        pkgs: crossPkgs: rec {
          smart-piano = pkgs.callPackage ./engine.nix {
            inherit self pkgs;
            stdenv = pkgs.clangStdenv;
          };
          cross-smart-piano = crossPkgs.callPackage ./engine.nix {
            inherit self;
            stdenv = crossPkgs.clangStdenv;
            pkgs = crossPkgs;
          };
          default = smart-piano;
          cross = cross-smart-piano;
        }
      );
      checks = systems (
        pkgs: _:
        let
          smart-piano = self.packages.${pkgs.stdenv.hostPlatform.system}.smart-piano;
        in
        {
          coverage = pkgs.clangStdenv.mkDerivation rec {
            pname = "coverage-check";
            src = self;
            nativeBuildInputs =
              (smart-piano.nativeBuildInputs or [ ])
              ++ (with pkgs; [
                lcov
                cmake
              ]);
            buildInputs = smart-piano.buildInputs or [ ];
            cmakeFlags = [
              "-DCMAKE_BUILD_TYPE=Debug"
              "-DCOVERAGE=ON"
            ];
            configurePhase = ''
              runHook preConfigure
              cmake -S . -B build ${toString cmakeFlags}
              runHook postConfigure
            '';
            buildPhase = ''
              runHook preBuild
              cmake --build build -j''${NIX_BUILD_CORES:-1}
              runHook postBuild
            '';
            checkPhase = ''
              runHook preCheck
              cd build
              ctest --output-on-failure
              runHook postCheck
            '';
            installPhase = ''
              runHook preInstall
              mkdir -p $out/
              cd build
              lcov --capture --directory . --output-file coverage.info
              lcov --remove coverage.info '/usr/*' --output-file coverage.info.filtered
              lcov --remove coverage.info.filtered '*/test/*' --output-file coverage.info.filtered
              lcov --remove coverage.info.filtered '*/build/_deps/*' --output-file coverage.info.filtered
              genhtml coverage.info.filtered --output-directory $out
              echo "Coverage report generated in $out"
              lcov --summary coverage.info.filtered
              runHook postInstall
            '';
            meta.description = "Code coverage report for Smart Piano Engine";
          };
        }
      );
      devShells = systems (
        pkgs: crossPkgs: {
          default =
            pkgs.mkShell.override
              {
                stdenv = pkgs.clangStdenv; # Clang instead of GCC
              }
              {
                packages = with pkgs; [
                  bashInteractive
                  clang-tools # Clang CLIs, including LSP
                  clang-uml # UML diagram generator
                  cmake-format # CMake formatter
                  cmake-language-server # Cmake LSP
                  # cppcheck # C++ Static analysis
                  doxygen # Documentation generator
                  # fluidsynth # JACK Synthesizer
                  lldb # Clang debug adapter
                  # neocmakelsp # CMake LSP
                  # qsynth # FluidSynth GUI
                  socat # Serial terminal for manual testing
                  # valgrind # Debugging and profiling
                ];
                nativeBuildInputs = self.packages.${pkgs.stdenv.hostPlatform.system}.smart-piano.nativeBuildInputs;
                buildInputs = self.packages.${pkgs.stdenv.hostPlatform.system}.smart-piano.buildInputs;
                # Export compile commands JSON for LSP and other tools
                shellHook = ''
                  mkdir --verbose build
                  cmake -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build
                '';
              };
        }
      );
    };
}
