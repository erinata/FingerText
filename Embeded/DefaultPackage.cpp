//This file is part of Fingertext, a notepad++ snippet plugin.
//
//Fingertext is released under MIT License.
//
//MIT license
//
//Copyright (C) 2011 by Tom Lam
//
//Permission is hereby granted, free of charge, to any person 
//obtaining a copy of this software and associated documentation 
//files (the "Software"), to deal in the Software without 
//restriction, including without limitation the rights to use, 
//copy, modify, merge, publish, distribute, sublicense, and/or 
//sell copies of the Software, and to permit persons to whom the 
//Software is furnished to do so, subject to the following 
//conditions:
//
//The above copyright notice and this permission notice shall be 
//included in all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
//OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
//NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
//HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
//WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
//DEALINGS IN THE SOFTWARE.

#include "DefaultPackage.h"

std::string getDefaultPackage() {
    return "time\nGLOBAL\n$[![(key)TIME:HH:mm:ss]!][>END<]!$[FingerTextData FingerTextData]@#\nshebang\nGLOBAL\n#!/usr/bin/env$[![]!][>END<]!$[FingerTextData FingerTextData]@#\nnpp\nGLOBAL\nNotepad++\n[>END<]!$[FingerTextData FingerTextData]@#\nmit\nGLOBAL\nMIT license\n\nCopyright (C) $[![(key)DATE:yyyy]!] by $[![{Your Name}]!]\n\nPermission is hereby granted, free of charge, to any person obtaining a copy\nof this software and associated documentation files (the \"Software\"), to deal\nin the Software without restriction, including without limitation the rights\nto use, copy, modify, merge, publish, distribute, sublicense, and/or sell\ncopies of the Software, and to permit persons to whom the Software is\nfurnished to do so, subject to the following conditions:\n\nThe above copyright notice and this permission notice shall be included in\nall copies or substantial portions of the Software.\n\nTHE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\nIMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\nFITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\nAUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\nLIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\nOUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN\nTHE SOFTWARE.\n\n$[![]!]\n\n[>END<]!$[FingerTextData FingerTextData]@#\nlorem\nGLOBAL\nLorem ipsum dolor sit amet, libero turpis non cras ligula, id commodo, aenean est in volutpat amet sodales, porttitor bibendum facilisi suspendisse, aliquam ipsum ante morbi sed ipsum mollis. Sollicitudin viverra, vel varius eget sit mollis. Commodo enim aliquam suspendisse tortor cum diam, commodo facilisis, rutrum et duis nisl porttitor, vel eleifend odio ultricies ut, orci in adipiscing felis velit nibh. Consectetuer porttitor feugiat vestibulum sit feugiat, voluptates dui eros libero. Etiam vestibulum at lectus.\nDonec vivamus. Vel donec et scelerisque vestibulum. Condimentum aliquam, mollit magna velit nec, tempor cursus vitae sit aliquet neque purus. Ultrices lacus proin conubia dictum tempus, tempor pede vitae faucibus, sem auctor, molestie diam dictum aliquam. Dolor leo, ridiculus est ut cubilia nec, fermentum arcu praesent, pede etiam. Tempor vestibulum turpis id ligula mi mattis. Eget arcu vitae mauris amet odio. Diam nibh diam, quam elit, libero nostra ut. Pellentesque vehicula. Eget sed, dapibus magna nulla nonummy commodo accumsan morbi, praesent volutpat vel id maecenas, morbi habitant sem in adipiscing mi erat, malesuada pretium tortor rutrum eu eros vel. Donec molestie, faucibus a amet commodo scelerisque libero massa. Sapien quam in eu vel nulla.\nIaculis et dui ullamcorper, non egestas condimentum dui phasellus. Sit non mattis a, leo in imperdiet erat nec pulvinar. Ornare massa justo cursus, convallis mauris interdum felis. Felis posuere metus, ornare pede montes, morbi urna sed temporibus non, nibh inceptos enim turpis natoque ac praesent. Litora vivamus veritatis vel nonummy, ut qui est pellentesque at alias, sed condimentum dapibus.\nRhoncus lacinia. Imperdiet nulla sem fringilla, purus enim amet, nascetur faucibus, adipiscing neque ut bibendum, at felis nec in. Mauris ultricies, et pede id potenti in nec, mi elit rhoncus ligula, mollis lacus congue scelerisque magna. Ultrices risus elit lectus nunc blandit quis, magna enim ipsum, nostra leo vestibulum quis nibh arcu sed. Amet a sagittis fringilla, massa vitae rhoncus, a magna curabitur in.[>END<]!$[FingerTextData FingerTextData]@#\nfilename\nGLOBAL\n$[![(key)FILENAME]!]$[![(key)EXTNAME]!][>END<]!$[FingerTextData FingerTextData]@#\nexecute\nGLOBAL\n$[![(run)$[![{type a command (e.g. dir) and hit TAB}]!]]!][>END<]!$[FingerTextData FingerTextData]@#\nevaluate\nGLOBAL\n$[![(eva)$[![{type a math expression (e.g. 1+1) and hit TAB}]!]]!][>END<]!$[FingerTextData FingerTextData]@#\ndirectory\nGLOBAL\n$[![(key)DIRECTORY]!][>END<] !$[FingerTextData FingerTextData]@#\ndate\nGLOBAL\n$[![(key)DATE:yyyy/MM/dd]!][>END<]!$[FingerTextData FingerTextData]@#\n";
}