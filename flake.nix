{
  description = "Nix flake C++23 development environment";
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  outputs =
    { self, nixpkgs }:
    let
      localSystems = [
        "aarch64-linux"
        "x86_64-linux"
        "aarch64-darwin"
      ];
      forAllSystems =
        f: nixpkgs.lib.genAttrs localSystems (system: f (import nixpkgs { inherit system; }));
      forAllSystemsWithCross =
        f:
        let
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
      packages = forAllSystems (
        pkgs:
        let
          smart-piano = pkgs.callPackage ./engine.nix {
            inherit self pkgs;
            stdenv = pkgs.clangStdenv;
          };
        in
        {
          inherit smart-piano;
          default = smart-piano;
        }
      );
      crossPackages = forAllSystemsWithCross (
        pkgs: crossPkgs:
        let
          cross-smart-piano = crossPkgs.callPackage ./engine.nix {
            inherit self;
            stdenv = crossPkgs.clangStdenv;
            pkgs = crossPkgs;
          };
        in
        {
          inherit cross-smart-piano;
          default = cross-smart-piano;
        }
      );
      checks = forAllSystems (pkgs: {
        coverage = pkgs.clangStdenv.mkDerivation {
          name = "coverage-check";
          src = self;
          nativeBuildInputs =
            (self.packages.${pkgs.stdenv.hostPlatform.system}.smart-piano.nativeBuildInputs or [ ])
            ++ [ pkgs.lcov ];
          buildInputs = (self.packages.${pkgs.stdenv.hostPlatform.system}.smart-piano.buildInputs or [ ]);
          configurePhase = ''
            cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON
          '';
          buildPhase = ''
            cmake --build build
          '';
          checkPhase = ''
            cmake --build build --target tests
            cmake --build build --target coverage
            llvm-cov report build/src/main -instr-profile=build/coverage.profdata -ignore-filename-regex='test/.*' > build/coverage.txt
            cat build/coverage.txt
            echo "Verifying functions coverage is > 90%"
            grep TOTAL build/coverage.txt | awk '{ if ($7 + 0 > 90) exit 0; else { print "Function coverage is " $7 "% (< 90%)"; exit 1 } }'
            echo "Verifying line coverage is > 90%"
            grep TOTAL build/coverage.txt | awk '{ if ($10 + 0 > 90) exit 0; else { print "Line coverage is " $10 "% (< 90%)"; exit 1 } }'
          '';
          installPhase = ''
            mkdir -p $out
            if [ -f "build/coverage.txt" ]; then
              cp build/coverage.txt $out/
            fi
            if [ -d "build/coverage" ]; then
              cp -R build/coverage $out/html
            fi
          '';
        };
      });
      devShells = forAllSystems (pkgs: {
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
      });
    };
}
