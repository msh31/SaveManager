#pragma once

enum PipelineError {
    EMPTY_INPUT = 1,    // recoverablej
    PROCESSING_FAILURE, // show msg
    INVALID_STATE       // unrecoverable
};
