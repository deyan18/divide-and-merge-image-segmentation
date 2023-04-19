# Image Segmentation using Divide and Merge Algorithm

This is an implementation of the Divide and Merge algorithm for image segmentation. The algorithm checks if a region is uniform, and if not, it divides it into four equal parts. After all possible divisions, it checks if adjacent or neighboring regions can be merged based on uniformity criteria. This algorithm commonly uses a quadtree for efficient processing.

## Examples

![Example 1](https://i.imgur.com/0qcvDua.png)
![Example 2](https://i.imgur.com/Yl2XmpN.png)
![Example 3](https://i.imgur.com/Vl7pQUq.png)

As observed , the algorithm performs better with some images than others. It is important to note that the results heavily depend on the values assigned to the variables, and therefore, further improvements can be made. Additionally, using only grayscale images makes it challenging to distinguish different parts of an object, although implementing color image segmentation would likely be more complex at my current level. Overall, despite encountering various difficulties, I found this project to be highly interesting and fulfilling as I was able to develop it independently.

## Usage

1. Clone the repository and navigate to the `Run` folder.
2. Run the program and provide the name of the image file when prompted. The image file should be located in the `Run` folder.
3. Optionally, you can modify the following parameters for image segmentation:

### Parameters for Image Division

- **Percentage of Allowed Errors for Division**: This parameter controls the number of allowed errors compared to the region's average intensity. A lower value results in more divisions, while a higher value results in fewer divisions. Recommended range: 5-10. Smaller values for images with high intensity variations, and larger values for simpler images.

- **Range of Allowed Intensity Difference for Division**: This parameter defines the range of intensity values considered valid for a pixel compared to the region's average intensity. Recommended range: around 5, to balance between false positives and false negatives.

### Parameters for Image Fusion

- **Percentage of Allowed Errors for Fusion**: This parameter limits the number of allowed errors in the merge of two regions. If the percentage of errors exceeds this value, the regions will not be merged. Recommended range: 15-30. Lower values for images with distinct regions or when aiming for more regions, and higher values for images where objects have similar intensity values and larger regions are desired.

- **Range of Allowed Intensity Difference for Fusion**: This parameter works similarly to the previous one, but for the fusion of multiple regions. Recommended range: 10-25, with the same considerations as before.

- **Node Division Limit**: This parameter sets a limit for the height or width of a region. If the height or width becomes smaller than this value, the division process will stop. Recommended range: 1-3. Smaller values for images with more details, and larger values for simpler images.

### Additional Option for Background Separation

- **Background Division Factor**: This parameter determines the factor by which the maximum size of a region will be divided to search for the next maximum. Recommended range: 1-4, smaller values for almost uniform backgrounds, and larger values for backgrounds with gradients or variations.

- **Background Separation Limit**: This parameter determines the number of times the loop will search for a new maximum. A value of 100 means the loop will continue until no new maximum is found. Any other value guarantees that the loop will execute that number of times. Recommended value: 100 or around 25 for images with smaller background regions.

4. Once the parameters are set, the program will execute the algorithm and save the results in the `Run` folder with the names `resultadoSegmentacion.bmp` and `resultadosSeparacion.bmp` if background separation is chosen.

Note: Sometimes the execution may seem to be stuck for a long time. Pressing the space key may resolve the issue.
