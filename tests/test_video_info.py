import deepframe

def test_get_video_info(video_url):
    info = deepframe.get_info(video_url)
    assert info["duration"] == 153600
    assert info["fps"] == 30.0
    assert info["width"] == 1280
    assert info["height"] == 720
    assert info["num_channels"] == 3
    assert info["num_frames"] == 300
    assert info["time_base_den"] == 15360
    assert info["time_base_num"] == 1
