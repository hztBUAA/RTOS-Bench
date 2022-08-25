{
  description = "A reproducible environment for rt-bench";
  inputs ={
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  }; 
  outputs = inputs:
    inputs.flake-utils.lib.eachDefaultSystem (system:
      let pkgs = inputs.nixpkgs.legacyPackages.${system}; in
    {
        devShell = pkgs.mkShell {
          buildInputs=[
            (pkgs.python3.withPackages (p: [p.numpy p.matplotlib]))
            pkgs.gcc
            pkgs.util-linux
            pkgs.gnugrep
            pkgs.coreutils
            pkgs.ps
            pkgs.cloc
            pkgs.doxygen
            pkgs.graphviz
            pkgs.gnumake
            pkgs.git
            pkgs.gnused
            pkgs.json_c
            pkgs.imagemagick
            pkgs.bash
          ];
        };
    }
    );
}
