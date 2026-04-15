# Install dependencies (Ubuntu)
sudo apt update
sudo apt install fuse3 libfuse3-dev pkg-config

# Clone repo
git clone <repo-url>
cd mini-unionfs

# Build
make

# Run
make run

# In another terminal → test
ls mnt

# Unmount
make unmount

![Test Results](image.png)