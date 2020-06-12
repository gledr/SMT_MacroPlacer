[![Build Status](https://travis-ci.com/gledr/SMT_MacroPlacer.svg?branch=master)](https://travis-ci.com/gledr/SMT_MacroPlacer)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
# SMT Macro Placer
VLSI MacroPlacer for System on Chip Design



Solving SoC Macro Placement using SMT Optimization. This version is based on Microsoft's Z3 solver.       We have also been using OptiMathSAT for some experiments. 



Our SMT Macro Placer is using parts of the OpenROAD project. To this end, we are very thankful for being able to reuse their LEF/DEF parser infrastructure. 



In oder to compare our results we have been using the Parquet floorplanning tool.  Therefore we could

also use OpenROADs Parquet build. Thank you very much for that.



The tool is under steady development. To this end, there may be lots of bugs and other non expected things. Next steps for us is to try different SMT coding strategies.



For any questions, do not hesitate to contact me, I will be glad to help.





