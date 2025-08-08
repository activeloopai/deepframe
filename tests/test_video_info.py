import deepframe

def test_get_video_info():
    info = deepframe.get_video_info("http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4")
    assert info["duration"] == 14315000
    assert info["fps"] == 24.0
    assert info["width"] == 1280
    assert info["height"] == 720
    assert info["num_channels"] == 3
    assert info["num_frames"] == 14315
    assert info["time_base_den"] == 24000
    assert info["time_base_num"] == 1
