import matplotlib.pyplot as plt
import sys

colors = ["#fc0000", "#f58402", "#66fffc"]
ErrorOneSigma = [1.00, 2.30, 3.53, 4.72, 5.89, 7.04]
ErrorTwoSigma = [4.00, 6.17, 8.02, 9.70, 11.3, 12.8]
ErrorThreeSigma = [9.00, 11.8, 14.2, 16.3, 18.2, 20.1]


class Point:
    def __init__(self, x, y, error):
        self.x = float(x)
        self.y = float(y)
        self.error = float(error)


def generate_image(path: str):
    with open(path, "r") as file:
        data = file.read().splitlines()
    degOfFreedom = int(data[0]) - 1
    title = data[1]
    x_label = data[2]
    y_label = data[3]
    pointsData = []
    for item in data[4:]:
        splitted = item.split(",")
        pointsData.append(Point(splitted[0], splitted[1], splitted[2]))

    oneSigma = [item for item in pointsData if item.error < ErrorOneSigma[degOfFreedom]]
    twoSigma = [
        item
        for item in pointsData
        if item.error > ErrorOneSigma[degOfFreedom]
        and item.error < ErrorTwoSigma[degOfFreedom]
    ]
    threeSigma = [
        item
        for item in pointsData
        if item.error > ErrorTwoSigma[degOfFreedom]
        #        and item.error < ErrorThreeSigma[degOfFreedom]
    ]
    plt.figure(figsize=(10, 6))
    for points, color in zip([threeSigma, twoSigma, oneSigma], colors):
        x = [item.x for item in points]
        y = [item.y for item in points]
        plt.scatter(x, y, color=color)

    # Add labels and title
    plt.xlabel(f"{x_label}")
    plt.ylabel(f"{y_label}")
    plt.title(f"{title}")

    # Save the plot as a PNG file
    plt.savefig(
        f"D:\STUDIA\Semiconductor-Studio\SemiconductorStudio\JunctionFitMaster\dat\MC\s.png"
    )


generate_image(
    "D:\STUDIA\Semiconductor-Studio\SemiconductorStudio\JunctionFitMaster\dat\MC\s.txt"
)
