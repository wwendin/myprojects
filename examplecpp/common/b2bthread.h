#pragma once
//
// Helpers for thread operations
//

#define gettid() syscall(__NR_gettid)
