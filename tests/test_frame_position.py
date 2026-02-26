"""
Test for frame position rounding bug.

The floating-point computation in extract_video_frames_from_video_at_url_():
    current_frame = int64_t((pts - start_pts) * time_base * frame_rate)
can truncate to the wrong frame number due to rounding errors
(e.g. 7.0 computes as 6.999999999999999, truncated to 6).

This causes "invalid frame position: X > Y" warnings, very slow access,
and all-zero ghost frames.
"""

import deepframe
import numpy as np

VIDEO_URL = "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4"


def test_single_frame_access_no_rounding_error():
    """Extracting a single frame at indices affected by float rounding
    should return a non-zero image.

    With time_base=1/24000 and fps=24, pts * (1/24000) * 24 produces
    values like 6.999999999999999 for frame 7, which truncates to 6.
    """
    # Indices known to trigger rounding with time_base=1/24000, fps=24
    rounding_indices = [7, 14, 28, 31, 56, 59, 62, 109]
    # Normal indices as sanity check (frame 0 is black in Big Buck Bunny)
    normal_indices = [100, 5000]

    for idx in rounding_indices + normal_indices:
        buf = deepframe.extract_frames(VIDEO_URL, [idx])
        arr = np.asarray(buf)
        assert arr.shape[0] == 1, f"frame[{idx}]: expected 1 frame, got {arr.shape[0]}"
        assert arr.mean() > 0, f"frame[{idx}]: got all-zero frame (rounding bug)"


def test_sequential_frame_values_are_unique():
    """Consecutive frames across a rounding boundary should have distinct
    content, not be duplicated from a wrong seek position."""
    # Frames 6-9 span the first rounding boundary (frame 7)
    indices = list(range(6, 10))
    buf = deepframe.extract_frames(VIDEO_URL, indices)
    arr = np.asarray(buf)
    assert arr.shape[0] == len(indices)

    for i in range(len(indices) - 1):
        assert not np.array_equal(
            arr[i], arr[i + 1]
        ), f"frame[{indices[i]}] and frame[{indices[i+1]}] are identical (likely seek error)"


def test_frame_count_matches_decodable_frames():
    """The last frames reported by get_info should be decodable
    and non-zero."""
    info = deepframe.get_info(VIDEO_URL)
    num_frames = int(info["num_frames"])

    # Check the last 20 frames
    start = num_frames - 20
    indices = list(range(start, num_frames))
    buf = deepframe.extract_frames(VIDEO_URL, indices)
    arr = np.asarray(buf)

    zero_frames = sum(1 for i in range(arr.shape[0]) if arr[i].mean() == 0)
    assert zero_frames == 0, (
        f"{zero_frames} of last {len(indices)} frames are all-zero "
        f"(ghost frames beyond actual video content)"
    )
