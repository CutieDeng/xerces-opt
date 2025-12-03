namespace array_detector {

class ArrayDetector;

}

namespace cutie_ns {

using namespace array_detector;

CutieErrorCode array_detect_execute(CUTIE_FUNC_ARGS);
CutieErrorCode array_detect_analysis(CUTIE_FUNC_ARGS);
CutieErrorCode trace_field_assignments(CUTIE_FUNC_ARGS, ArrayDetector* detector);

} // namespace cutie_ns
