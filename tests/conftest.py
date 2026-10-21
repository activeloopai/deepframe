import pytest

# Single source of truth for the test video. If this host goes away, update
# the URL here and re-derive the expected values in test_video_info.py and
# test_frame_position.py from deepframe.get_info() on the new file.
#
# Big Buck Bunny (c) Blender Foundation, CC BY 3.0, 10 s 720p H.264 clip.
VIDEO_URL = "https://test-videos.co.uk/vids/bigbuckbunny/mp4/h264/720/Big_Buck_Bunny_720_10s_1MB.mp4"


@pytest.fixture(scope="session")
def video_url():
    return VIDEO_URL
