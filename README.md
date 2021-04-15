## Overview

Source repository for the `C++` analysis code from an undergraduate research project in astrophysics focusing on the "Core Accretion" and "Competetive Accretion" theories of stellar formation in the context of [FLASH code](http://flash.uchicago.edu/site/flashcode/) simulations of star formation in the early universe. This code was written in 2014 at UT-Austin, and was migrated from BitBucket in May 2018.

It's unlikely that this code will be functional out-of-the-box, but pieces of it may be helpful for someone who's trying to analyze checkpoint file mesh data from [FLASH code](http://flash.uchicago.edu/site/flashcode/) radiation-hydrodynamic simulations with Adaptive Mesh Refinement and/or Lagrangian sink particles.

For now the header files in `Include/` are probably the quickest way to get a sense of the code structure. Feel free to reach out and I'll do my best to answer any questions.

## Stellar Cores

Parts of the code utilize [libtourtre: A Countour Tree Library](http://graphics.cs.ucdavis.edu/~sdillard/libtourtre/doc/html/) to build countour trees from the gravitatial potential field. Stellar cores are then defined as containing the cells mapping to the connected component directly "above" the node of the corresponding stellar sink particle.

`CoreAnalyzer::altFindCoreRegion()` uses a seeded region growing method to attempt to find topologically equivalent core regions in a different, more efficient manner.

In theory, these methods should have produced identical cell-to-region mappings, although in practice they produce slightly different results. They did both produce very low-mass stellar initial mass functions (IMFs) that were uncorrelated with the final stellar mass function, suggesting that competetive accretion was much more descriptive of the stellar formation processes in these simulations. This is in agreement with the findings of the 2015 paper based on the same simulations: *Star formation in the first galaxies - III* (linked below).

## Papers

For more information about the simulations, and their scientific purpose and context, consult the following papers:

### Before this code was written:

- [Star Formation in the First Galaxies I: Collapse Delayed by Lyman-Werner Radiation](https://arxiv.org/abs/1205.3835) - Chalence Safranek-Shrader, Meghann Agarwal, Christoph Federrath, Anshu Dubey, Milos Milosavljevic, Volker Bromm
- [Star Formation in the First Galaxies - II: Clustered Star Formation and the Influence of Metal Line Cooling](https://arxiv.org/abs/1307.1982) - Chalence Safranek-Shrader, Milos Milosavljevic, Volker Bromm
- [Formation of the First Low-Mass Stars from Cosmological Initial Conditions](https://arxiv.org/abs/1401.0540) - Chalence Safranek-Shrader, Milos Milosavljevic, Volker Bromm

### After this code was written:

- [Star formation in the first galaxies - III. Formation, evolution, and characteristics of the first stellar cluster](https://arxiv.org/abs/1501.03212) - Chalence Safranek-Shrader, Michael Montgomery, Milos Milosavljevic, Volker Bromm

### Background on the core and competetive theories of star formation:

- [Models for the Formation of Massive Stars](https://arxiv.org/abs/0712.0828) - Mark R. Krumholz, Ian A. Bonnell


