{
  description = "Nix flake C++23 development environment";
  nixConfig = {
    extra-substituters = [ "https://cache.garnix.io" ];
    extra-trusted-public-keys = [ "cache.garnix.io:CTFPyKSLcx5RMJKfLo5EEPUObbA78b0YQ2DTCJXqr9g=" ];
  };
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  outputs =
    { self, nixpkgs }:
    let
      localSystems = [
        "x86_64-linux" # "aarch64-linux"
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
            ++ [
              pkgs.lcov
              pkgs.jq
            ];
          buildInputs = (self.packages.${pkgs.stdenv.hostPlatform.system}.smart-piano.buildInputs or [ ]);
          configurePhase = ''
            cmake -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON -GNinja -S . -B build
          '';
          buildPhase = ''
            cmake --build build -j16
          '';
          checkPhase = ''
            cmake --build build --target coverage -j1 # Multithread breaks tests
            # Text report for the CI comment
            llvm-cov report build/src/main -instr-profile=build/coverage.profdata -ignore-filename-regex='test/.*' > build/coverage.txt
            cat build/coverage.txt

            # JSON export for threshold verification
            llvm-cov export build/src/main -instr-profile=build/coverage.profdata -ignore-filename-regex='test/.*' --summary-only > build/coverage.json

            # Extract totals
            FUNCTIONS=$(jq '.data[0].totals.functions.percent' build/coverage.json)
            LINES=$(jq '.data[0].totals.lines.percent' build/coverage.json)

            echo "#### Coverage thresholds check ####"
            echo "Function coverage: $FUNCTIONS% (Required: 90%)"
            echo "Line coverage:     $LINES% (Required: 80%)"

            # Check thresholds (using jq for float comparison)
            jq -e '.data[0].totals.functions.percent >= 90' build/coverage.json > /dev/null || (echo "FAILED: Function coverage is below 90%" && exit 1)
            jq -e '.data[0].totals.lines.percent >= 80' build/coverage.json > /dev/null || (echo "FAILED: Line coverage is below 80%" && exit 1)
          '';
          installPhase = ''
            mkdir -p $out
            [ -f "build/coverage.txt" ] && cp build/coverage.txt $out/
            [ -d "build/coverage" ] && cp -R build/coverage $out/html
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
                doxygen # Documentation generator
                fluidsynth # JACK Synthesizer
                lldb # Clang debug adapter
                qsynth # FluidSynth GUI
                socat # Serial terminal for manual testing
                valgrind # Debugging and profiling
              ];
              nativeBuildInputs = self.packages.${pkgs.stdenv.hostPlatform.system}.smart-piano.nativeBuildInputs;
              buildInputs = self.packages.${pkgs.stdenv.hostPlatform.system}.smart-piano.buildInputs;
              # Export compile commands JSON for LSP and other tools
              shellHook = ''
                mkdir --verbose build
                cmake -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -GNinja -S . -B build
              '';
            };
      });
    };
}
