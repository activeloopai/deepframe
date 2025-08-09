import deepframe
import numpy as np

def test_video_fetch_frames():
    buf = deepframe.extract_frames("http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4", slice(None, None, 5), 100)
    arr = np.asarray(buf)

    assert arr.shape == (20, 720, 1280, 3)


