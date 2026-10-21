"""
Test for frame position rounding bug.

The floating-point computation in extract_video_frames_from_video_at_url_():
    current_frame = int64_t((pts - start_pts) * time_base * frame_rate)
can truncate to the wrong frame number due to rounding errors
(e.g. 31.0 computes as 30.999999999999996, truncated to 30).

This causes "invalid frame position: X > Y" warnings, very slow access,
and all-zero ghost frames.
"""

import deepframe
import numpy as np


def test_single_frame_access_no_rounding_error(video_url):
    """Extracting a single frame at indices affected by float rounding
    should return a non-zero image.

    With time_base=1/15360 and fps=30 (512 pts per frame),
    pts * (1/15360) * 30 produces values like 30.999999999999996 for
    frame 31, which truncates to 30.
    """
    # Indices known to trigger rounding with time_base=1/15360, fps=30
    rounding_indices = [31, 62, 111, 123, 124, 207, 222, 237]
    # Normal indices as sanity check
    normal_indices = [100, 250]

    for idx in rounding_indices + normal_indices:
        buf = deepframe.extract_frames(video_url, [idx])
        arr = np.asarray(buf)
        assert arr.shape[0] == 1, f"frame[{idx}]: expected 1 frame, got {arr.shape[0]}"
        assert arr.mean() > 0, f"frame[{idx}]: got all-zero frame (rounding bug)"


def test_sequential_frame_values_are_unique(video_url):
    """Consecutive frames across a rounding boundary should have distinct
    content, not be duplicated from a wrong seek position."""
    # Frames 30-33 span the first rounding boundary (frame 31)
    indices = list(range(30, 34))
    buf = deepframe.extract_frames(video_url, indices)
    arr = np.asarray(buf)
    assert arr.shape[0] == len(indices)

    for i in range(len(indices) - 1):
        assert not np.array_equal(
            arr[i], arr[i + 1]
        ), f"frame[{indices[i]}] and frame[{indices[i+1]}] are identical (likely seek error)"


def test_frame_count_matches_decodable_frames(video_url):
    """The last frames reported by get_info should be decodable
    and non-zero."""
    info = deepframe.get_info(video_url)
    num_frames = int(info["num_frames"])

    # Check the last 20 frames
    start = num_frames - 20
    indices = list(range(start, num_frames))
    buf = deepframe.extract_frames(video_url, indices)
    arr = np.asarray(buf)

    zero_frames = sum(1 for i in range(arr.shape[0]) if arr[i].mean() == 0)
    assert zero_frames == 0, (
        f"{zero_frames} of last {len(indices)} frames are all-zero "
        f"(ghost frames beyond actual video content)"
    )
