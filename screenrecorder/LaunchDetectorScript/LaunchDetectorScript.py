import argparse
from pathlib import Path
import torch.nn as nn
import torch
from PIL import Image
import os
from torchvision import transforms as T
import shutil
import sys

class CropBottomPercent:
    def __init__(self, percentage):
        self.percentage = percentage

    def __call__(self, image):
        width, height = image.size
        crop_height = int(height * self.percentage)

        return image.crop((0, 0, width, height - crop_height))

#Creating a class that loads the dataset from the directories
class launchDetector(torch.utils.data.Dataset):
    def __init__(self, path, train = True, transforms = None):
        self.path = path
        self.transforms = transforms
        self.images = list(sorted(os.listdir(path)))

    def __getitem__(self, index):
        image_path = os.path.join(self.path, self.images[index])
        image = Image.open(image_path).convert("RGB")

        if self.transforms is not None:
            image = self.transforms(image)

        return image

    def __len__(self):
        return len(self.images)


#Create the transforms that will with which the data will be preprocessed
img_transforms = T.Compose([CropBottomPercent(0.05),
                            T.Resize((224,224)),
                            T.ToTensor(),
                            T.Normalize([0.485, 0.456, 0.406],
                                        [0.229, 0.224, 0.225])])

## CNN Module
class CNN(nn.Module):
    def __init__(self):
        super(CNN, self).__init__()
        self.max_pool1 = nn.MaxPool2d(kernel_size = 4, stride = 2)
    
    # Progresses data across layers    
    def forward(self, x):
        output = self.max_pool1(x)
        return output

def has_non_image_files(file_path):
    for file in os.listdir(file_path):
        if not file.endswith('.jpg') and not file.endswith('.png'):
            return True
    return False

def clean_directory(folder_path):
    # Clean non-image files from folder
    for file in os.listdir(folder_path):                    # Loop through all files in directory
        file_path = os.path.join(folder_path, file)          # Get full file path
        try:
            im = Image.open(file_path)                       # Try to open file as image
        except:
            non_image_folder = folder_path + "_non_images"    # If file is not image, create folder for non-images
            if not os.path.exists(non_image_folder):
                os.mkdir(non_image_folder)                        # Create folder if it doesn't exist already
            shutil.move(file_path, os.path.join(non_image_folder, file))  # Move non-image file to non-image folder
            continue
    
def imageSimilarityIndexGenerator(img_1, img_2):
    img_1 = torch.reshape(img_1, (-1,))
    img_2 = torch.reshape(img_2, (-1,))
    cos = nn.CosineSimilarity(dim=0, eps=1e-6)
    return cos(img_1, img_2).numpy()

def launchFrameDetector(cosine_similarity_list, file_names):
    similarity_threshold = 0.999
    for similarity_value in reversed(range(len(cosine_similarity_list))):
        if float(cosine_similarity_list[similarity_value]) < similarity_threshold:
            return file_names[similarity_value + 1]

parser = argparse.ArgumentParser(description='This script analyzes a directory of images and returns the name of the image that is most similar to the reference image.\n\n'
                                             'Any files that are not images in the folder given in file_path will be removed from the folder and moved to a folder in the parent folder with the name "folder_path" + "_non_images".\n')
parser.add_argument("file_path", type=Path, help='path to the directory containing the images to be analyzed')

p = parser.parse_args()
images_path = str(p.file_path)

if has_non_image_files(images_path):
    clean_directory(images_path)
    print("Non-image files were found in the directory and have been moved to a folder called " + images_path + "_non_images")

# Get the list of image file names
file_names = [f for f in os.listdir(images_path) if f.endswith('.jpg') or f.endswith('.png')]

train_dataset = launchDetector(images_path, transforms=img_transforms)

model = CNN()
pooled_tensors = []
for tensor in train_dataset:
    pooled_tensors.append(model.forward(tensor))

launch_cosine_similarity = []
for current_image_index in range(len(pooled_tensors)-1):
    launch_cosine_similarity.append(imageSimilarityIndexGenerator(train_dataset[current_image_index],train_dataset[current_image_index+1]))

sys.stdout.write(str(launchFrameDetector(launch_cosine_similarity, file_names)))
