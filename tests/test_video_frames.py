import deepframe
import numpy as np

def test_video_fetch_frames(video_url):
    buf = deepframe.extract_frames(video_url, slice(None, None, 5), 100)
    arr = np.asarray(buf)

    assert arr.shape == (20, 720, 1280, 3)
