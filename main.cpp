#include <opencv2/opencv.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <thread>

class BlueToolGame {
public:
    BlueToolGame() : score(0), gameDuration(30) {
        cap = initializeCamera();
        initializeBackground();
        lastRedBallUpdateTime = std::chrono::system_clock::now();
        sniperPosition = cv::Point(320, 240); // Initial sniper position
    }

    void run() {
        auto startTime = std::chrono::high_resolution_clock::now();

        while (true) {
            cv::Mat frame;
            cap >> frame;
            if (frame.empty()) {
                std::cerr << "Error: Could not read frame." << std::endl;
                break;
            }

            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = currentTime - startTime;

            int remainingTime = gameDuration - static_cast<int>(elapsed.count());

            if (remainingTime <= 0) {
                // Game over if time runs out
                break;
            }

            // Detect the blue tool in the frame and get its position
            cv::Point blueToolPosition = detectBlueTool(frame);

            // Update the sniper position based on the blue tool position
            if (blueToolPosition.x != -1 && blueToolPosition.y != -1) {
                sniperPosition = blueToolPosition;
            }

            // Draw the sniper cross at the updated position
            drawSniperCross(frame, sniperPosition, 20, cv::Scalar(0, 0, 255));

            // Update the game state, including score, red balls, and red ball interactions
            score = updateRedBalls(frame, score);

            displayGame(frame, score, remainingTime);

            // Exit the loop when the 'q' key is pressed or when the time runs out
            if (cv::waitKey(1) == 'q' || remainingTime <= 0) {
                break;
            }
        }

        // Release resources
        cap.release();
        cv::destroyAllWindows();
    }
    int getScore() const {
        return score;
    }

private:
    cv::VideoCapture cap;
    cv::Mat background;
    int score;
    int gameDuration;
    std::vector<cv::Point> redBalls;
    std::chrono::system_clock::time_point lastRedBallUpdateTime;
    cv::Point sniperPosition;

    cv::VideoCapture initializeCamera() {
        cv::VideoCapture cap(0); // 0 for the default camera
        if (!cap.isOpened()) {
            std::cerr << "Error: Camera not found or couldn't be opened." << std::endl;
            exit(-1);
        }
        return cap;
    }

    void initializeBackground() {
        background = cv::imread("ubuntu.jpg");
        cv::resize(background, background, cv::Size(640, 480));
    }

    cv::Point detectBlueTool(const cv::Mat& frame) {
        cv::Mat hsvImage;
        cv::cvtColor(frame, hsvImage, cv::COLOR_BGR2HSV);

        // Define the lower and upper bounds for blue color
        cv::Scalar lowerBlue = cv::Scalar(90, 50, 50);    // Adjust lower bound for hue and saturation
        cv::Scalar upperBlue = cv::Scalar(130, 255, 255); // Keep upper bound for hue and saturation

        // Create a binary mask for blue color
        cv::Mat blueMask;
        cv::inRange(hsvImage, lowerBlue, upperBlue, blueMask);

        // Find the centroid of the blue object (tool)
        cv::Moments moments = cv::moments(blueMask, true);
        cv::Point blueToolPosition(moments.m10 / moments.m00, moments.m01 / moments.m00);
        return blueToolPosition;
    }

    void drawSniperCross(cv::Mat& frame, const cv::Point& center, int size, cv::Scalar color) {
        int halfSize = size / 2;
        cv::line(frame, cv::Point(center.x - halfSize, center.y), cv::Point(center.x + halfSize, center.y), color, 2);
        cv::line(frame, cv::Point(center.x, center.y - halfSize), cv::Point(center.x, center.y + halfSize), color, 2);
    }

    int updateRedBalls(cv::Mat& frame, int score) {
        // Remove any red balls that have been touched by the sniper
        for (auto it = redBalls.begin(); it != redBalls.end();) {
            cv::Point redBallCenter = *it;
            int distance = cv::norm(sniperPosition - redBallCenter);

            if (distance <= 20) { // Adjust the distance threshold as needed
                it = redBalls.erase(it);
                score++;
            } else {
                ++it;
            }
        }

        // Check if it's time to update the red balls
        auto currentTime = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastRedBallUpdateTime;

        if (elapsed.count() >= 3.0) { // Update red balls every 3 seconds
            generateRandomRedBall(frame);
            lastRedBallUpdateTime = currentTime;
        }

        return score;
    }

    void generateRandomRedBall(cv::Mat& frame) {
        // Generate random x and y coordinates for the red ball
        int maxX = frame.cols;
        int maxY = frame.rows;
        int randomX = std::rand() % maxX;
        int randomY = std::rand() % maxY;
        cv::Point redBall(randomX, randomY);

        // Add the red ball to the list
        redBalls.push_back(redBall);
    }

    void displayGame(cv::Mat& frame, int score, int remainingTime) {
        for (const auto& redBall : redBalls) {
            cv::circle(frame, redBall, 10, cv::Scalar(0, 0, 255), -1);
        }

        cv::putText(frame, "Score: " + std::to_string(score), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
        cv::putText(frame, "Compteur: " + std::to_string(remainingTime), cv::Point(frame.cols - 120, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);

        cv::imshow("Blue Tool Game", frame);
    }
};

int main() {
    BlueToolGame game;
    game.run();

    // Print the final score when the game is over
    std::cout << "Game Over! Final Score: " << game.getScore() << std::endl;

    return 0;
}
