#pragma once

#include "prelude.hh"
#include "context.hh"
#include "state.hh"

namespace array_detector {

using namespace cutie_ns;

class ArrayDetector;
CutieErrorCode analyze_field_assignments_in_functions(ArrayDetector &self, CUTIE_FUNC_ARGS);

} // namespace array_detector

namespace cutie_ns {

using namespace array_detector;

CutieErrorCode collect_all_types_and_fields(CUTIE_FUNC_ARGS, ArrayDetector* detector);

CutieErrorCode array_detect_execute(CUTIE_FUNC_ARGS);
CutieErrorCode array_detect_analysis(CUTIE_FUNC_ARGS);
CutieErrorCode trace_field_assignments(CUTIE_FUNC_ARGS, ArrayDetector* detector);

} // namespace cutie_ns
