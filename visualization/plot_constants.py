COLOR_CYCLE_SIZE = 8
NREPEATS = 5

# Input directories. For weak scaling, [1] is by number of Fourier transforms,
# [2] is by size of Fourier transforms.
TRILLIUM_SS_PREFIX = "../logs/scaling/trillium/strong-scaling"
TRILLIUM_WS_PREFIX = ["../logs/scaling/trillium/weak-scaling/1-number",
                      "../logs/scaling/trillium/weak-scaling/2-size"]
NIAGARA_SS_PREFIX = "../logs/scaling/niagara/strong-scaling"
NIAGARA_WS_PREFIX = ["../logs/scaling/niagara/weak-scaling/1-number",
                     "../logs/scaling/niagara/weak-scaling/2-size"]

TRILLIUM_TIMELOG_BENCHMARKS = {
    "strong": {
        "omp": {
            "dirs": [f"{TRILLIUM_SS_PREFIX}/timelog_948513", f"{TRILLIUM_SS_PREFIX}/timelog_948517"],
            "labels": ["FFT-based with OpenMP", "Feedforward with OpenMP"]
        },
        "mpi": {
            "dirs": [f"{TRILLIUM_SS_PREFIX}/timelog_948540", f"{TRILLIUM_SS_PREFIX}/timelog_948567",
                     f"{TRILLIUM_SS_PREFIX}/timelog_948549", f"{TRILLIUM_SS_PREFIX}/timelog_948599"],
            "labels": ["FFT-based with in-node MPI", "FFT-based with off-node MPI",
                       "Feedforward with in-node MPI", "Feedforward with off-node MPI"]
        },
        "hybrid": {
            "dirs": [f"{TRILLIUM_SS_PREFIX}/timelog_949250", f"{TRILLIUM_SS_PREFIX}/timelog_953344"],
            "labels": ["FFT-based with OpenMP+MPI", "Feedforward with OpenMP+MPI"]
        }
    },
    "weak": {
        "omp": {
            "dirs": [f"{TRILLIUM_WS_PREFIX[0]}/timelog_1037929", f"{TRILLIUM_WS_PREFIX[0]}/timelog_1038315",
                     f"{TRILLIUM_WS_PREFIX[1]}/timelog_954108", f"{TRILLIUM_WS_PREFIX[1]}/timelog_953899"],
            "labels": ["FFT-based with OpenMP [1]", "Feedforward with OpenMP [1]",
                       "FFT-based with OpenMP [2]", "Feedforward with OpenMP [2]"]
        },
        "mpi": {
            "dirs": [f"{TRILLIUM_WS_PREFIX[0]}/timelog_1041500", f"{TRILLIUM_WS_PREFIX[0]}/timelog_1043287",
                     f"{TRILLIUM_WS_PREFIX[0]}/timelog_1041501", f"{TRILLIUM_WS_PREFIX[0]}/timelog_1043288",
                     f"{TRILLIUM_WS_PREFIX[1]}/timelog_958163", f"{TRILLIUM_WS_PREFIX[1]}/timelog_956661",
                     f"{TRILLIUM_WS_PREFIX[1]}/timelog_958121", f"{TRILLIUM_WS_PREFIX[1]}/timelog_954214"],
            "labels": ["FFT-based with in-node MPI [1]", "FFT-based with off-node MPI [1]",
                       "Feedforward with in-node MPI [1]", "Feedforward with off-node MPI [1]",
                       "FFT-based with in-node MPI [2]", "FFT-based with off-node MPI [2]",
                       "Feedforward with in-node MPI [2]", "Feedforward with off-node MPI [2]"]
        },
        "hybrid": {
            "dirs": [f"{TRILLIUM_WS_PREFIX[0]}/timelog_1041832", f"{TRILLIUM_WS_PREFIX[0]}/timelog_1041810",
                     f"{TRILLIUM_WS_PREFIX[1]}/timelog_1037638", f"{TRILLIUM_WS_PREFIX[1]}/timelog_983526"],
            "labels": ["FFT-based with OpenMP+MPI [1]", "Feedforward with OpenMP+MPI [1]",
                       "FFT-based with OpenMP+MPI [2]", "Feedforward with OpenMP+MPI [2]"]
        }
    }
}

NIAGARA_TIMELOG_BENCHMARKS = {
    "strong": {
        "omp": {
            "dirs": [f"{NIAGARA_SS_PREFIX}/timelog_12384", f"{NIAGARA_SS_PREFIX}/timelog_12380"],
            "labels": ["FFT-based with OpenMP", "Feedforward with OpenMP"]
        },
        "mpi": {
            "dirs": [f"{NIAGARA_SS_PREFIX}/timelog_5607", f"{NIAGARA_SS_PREFIX}/timelog_2561",
                     f"{NIAGARA_SS_PREFIX}/timelog_2560", f"{NIAGARA_SS_PREFIX}/timelog_2562"],
            "labels": ["FFT-based with in-node MPI", "FFT-based with off-node MPI",
                       "Feedforward with in-node MPI", "Feedforward with off-node MPI"]
        },
        "hybrid": {
            "dirs": [f"{NIAGARA_SS_PREFIX}/timelog_12388", f"{NIAGARA_SS_PREFIX}/timelog_12390"],
            "labels": ["FFT-based with OpenMP+MPI", "Feedforward with OpenMP+MPI"]
        }
    },
    "weak": {
        "omp": {
            "dirs": [f"{NIAGARA_WS_PREFIX[0]}/timelog_2249", f"{NIAGARA_WS_PREFIX[0]}/timelog_2250",
                     f"{NIAGARA_WS_PREFIX[1]}/timelog_12392", f"{NIAGARA_WS_PREFIX[1]}/timelog_12391"],
            "labels": ["FFT-based with OpenMP [1]", "Feedforward with OpenMP [1]",
                       "FFT-based with OpenMP [2]", "Feedforward with OpenMP [2]"]
        },
        "mpi": {
            "dirs": [f"{NIAGARA_WS_PREFIX[0]}/timelog_2260", f"{NIAGARA_WS_PREFIX[0]}/timelog_2456",
                     f"{NIAGARA_WS_PREFIX[0]}/timelog_2261", f"{NIAGARA_WS_PREFIX[0]}/timelog_2457",
                     f"{NIAGARA_WS_PREFIX[1]}/timelog_12400", f"{NIAGARA_WS_PREFIX[1]}/timelog_12394",
                     f"{NIAGARA_WS_PREFIX[1]}/timelog_12399", f"{NIAGARA_WS_PREFIX[1]}/timelog_12393"],
            "labels": ["FFT-based with in-node MPI [1]", "FFT-based with off-node MPI [1]",
                       "Feedforward with in-node MPI [1]", "Feedforward with off-node MPI [1]",
                       "FFT-based with in-node MPI [2]", "FFT-based with off-node MPI [2]",
                       "Feedforward with in-node MPI [2]", "Feedforward with off-node MPI [2]"]
        },
        "hybrid": {
            "dirs": [f"{NIAGARA_WS_PREFIX[0]}/timelog_2467", f"{NIAGARA_WS_PREFIX[0]}/timelog_2468",
                     f"{NIAGARA_WS_PREFIX[1]}/timelog_13283", f"{NIAGARA_WS_PREFIX[1]}/timelog_13282"],
            "labels": ["FFT-based with OpenMP+MPI [1]", "Feedforward with OpenMP+MPI [1]",
                       "FFT-based with OpenMP+MPI [2]", "Feedforward with OpenMP+MPI [2]"]
        }
    }
}

TRILLIUM_MEMLOG_BENCHMARKS = {
    "strong": {
        "omp": {
            "dirs": [f"{TRILLIUM_SS_PREFIX}/memlog_948513", f"{TRILLIUM_SS_PREFIX}/memlog_948517"],
            "labels": ["FFT-based with OpenMP", "Feedforward with OpenMP"]
        },
        "mpi": {
            "dirs": [f"{TRILLIUM_SS_PREFIX}/memlog_948540", f"{TRILLIUM_SS_PREFIX}/memlog_948567",
                     f"{TRILLIUM_SS_PREFIX}/memlog_948549", f"{TRILLIUM_SS_PREFIX}/memlog_948599"],
            "labels": ["FFT-based with in-node MPI", "FFT-based with off-node MPI",
                       "Feedforward with in-node MPI", "Feedforward with off-node MPI"]
        },
        "hybrid": {
            "dirs": [f"{TRILLIUM_SS_PREFIX}/memlog_949250", f"{TRILLIUM_SS_PREFIX}/memlog_953344"],
            "labels": ["FFT-based with OpenMP+MPI", "Feedforward with OpenMP+MPI"]
        }
    },
    "weak": {
        "omp": {
            "dirs": [f"{TRILLIUM_WS_PREFIX[0]}/memlog_1037929", f"{TRILLIUM_WS_PREFIX[0]}/memlog_1038315",
                     f"{TRILLIUM_WS_PREFIX[1]}/memlog_954108", f"{TRILLIUM_WS_PREFIX[1]}/memlog_953899"],
            "labels": ["FFT-based with OpenMP [1]", "Feedforward with OpenMP [1]",
                       "FFT-based with OpenMP [2]", "Feedforward with OpenMP [2]"]
        },
        "mpi": {
            "dirs": [f"{TRILLIUM_WS_PREFIX[0]}/memlog_1041500", f"{TRILLIUM_WS_PREFIX[0]}/memlog_1043287",
                     f"{TRILLIUM_WS_PREFIX[0]}/memlog_1041501", f"{TRILLIUM_WS_PREFIX[0]}/memlog_1043288",
                     f"{TRILLIUM_WS_PREFIX[1]}/memlog_958163", f"{TRILLIUM_WS_PREFIX[1]}/memlog_956661",
                     f"{TRILLIUM_WS_PREFIX[1]}/memlog_958121", f"{TRILLIUM_WS_PREFIX[1]}/memlog_954214"],
            "labels": ["FFT-based with in-node MPI [1]", "FFT-based with off-node MPI [1]",
                       "Feedforward with in-node MPI [1]", "Feedforward with off-node MPI [1]",
                       "FFT-based with in-node MPI [2]", "FFT-based with off-node MPI [2]",
                       "Feedforward with in-node MPI [2]", "Feedforward with off-node MPI [2]"]
        },
        "hybrid": {
            "dirs": [f"{TRILLIUM_WS_PREFIX[0]}/memlog_1041832", f"{TRILLIUM_WS_PREFIX[0]}/memlog_1041810",
                     f"{TRILLIUM_WS_PREFIX[1]}/memlog_1037638", f"{TRILLIUM_WS_PREFIX[1]}/memlog_983526"],
            "labels": ["FFT-based with OpenMP+MPI [1]", "Feedforward with OpenMP+MPI [1]",
                       "FFT-based with OpenMP+MPI [2]", "Feedforward with OpenMP+MPI [2]"]
        }
    }
}

NIAGARA_MEMLOG_BENCHMARKS = {
    "strong": {
        "omp": {
            "dirs": [f"{NIAGARA_SS_PREFIX}/memlog_12384", f"{NIAGARA_SS_PREFIX}/memlog_12380"],
            "labels": ["FFT-based with OpenMP", "Feedforward with OpenMP"]
        },
        "mpi": {
            "dirs": [f"{NIAGARA_SS_PREFIX}/memlog_5607", f"{NIAGARA_SS_PREFIX}/memlog_2561",
                     f"{NIAGARA_SS_PREFIX}/memlog_2560", f"{NIAGARA_SS_PREFIX}/memlog_2562"],
            "labels": ["FFT-based with in-node MPI", "FFT-based with off-node MPI",
                       "Feedforward with in-node MPI", "Feedforward with off-node MPI"]
        },
        "hybrid": {
            "dirs": [f"{NIAGARA_SS_PREFIX}/memlog_12388", f"{NIAGARA_SS_PREFIX}/memlog_12390"],
            "labels": ["FFT-based with OpenMP+MPI", "Feedforward with OpenMP+MPI"]
        }
    },
    "weak": {
        "omp": {
            "dirs": [f"{NIAGARA_WS_PREFIX[0]}/memlog_2249", f"{NIAGARA_WS_PREFIX[0]}/memlog_2250",
                     f"{NIAGARA_WS_PREFIX[1]}/memlog_12392", f"{NIAGARA_WS_PREFIX[1]}/memlog_12391"],
            "labels": ["FFT-based with OpenMP [1]", "Feedforward with OpenMP [1]",
                       "FFT-based with OpenMP [2]", "Feedforward with OpenMP [2]"]
        },
        "mpi": {
            "dirs": [f"{NIAGARA_WS_PREFIX[0]}/memlog_2260", f"{NIAGARA_WS_PREFIX[0]}/memlog_2456",
                     f"{NIAGARA_WS_PREFIX[0]}/memlog_2261", f"{NIAGARA_WS_PREFIX[0]}/memlog_2457",
                     f"{NIAGARA_WS_PREFIX[1]}/memlog_12400", f"{NIAGARA_WS_PREFIX[1]}/memlog_12394",
                     f"{NIAGARA_WS_PREFIX[1]}/memlog_12399", f"{NIAGARA_WS_PREFIX[1]}/memlog_12393"],
            "labels": ["FFT-based with in-node MPI [1]", "FFT-based with off-node MPI [1]",
                       "Feedforward with in-node MPI [1]", "Feedforward with off-node MPI [1]",
                       "FFT-based with in-node MPI [2]", "FFT-based with off-node MPI [2]",
                       "Feedforward with in-node MPI [2]", "Feedforward with off-node MPI [2]"]
        },
        "hybrid": {
            "dirs": [f"{NIAGARA_WS_PREFIX[0]}/memlog_2467", f"{NIAGARA_WS_PREFIX[0]}/memlog_2468",
                     f"{NIAGARA_WS_PREFIX[1]}/memlog_13283", f"{NIAGARA_WS_PREFIX[1]}/memlog_13282"],
            "labels": ["FFT-based with OpenMP+MPI [1]", "Feedforward with OpenMP+MPI [1]",
                       "FFT-based with OpenMP+MPI [2]", "Feedforward with OpenMP+MPI [2]"]
        }
    }
}
