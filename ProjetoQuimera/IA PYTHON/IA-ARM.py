import cv2
import mediapipe as mp
import numpy as np
import serial

mp_drawing = mp.solutions.drawing_utils
mp_hands = mp.solutions.hands
mp_pose = mp.solutions.pose

thumb_index_threshold = 0.05

ser = serial.Serial('COM5', 9600)


def calculate_angle(a, b, c):
    a = np.array(a)
    b = np.array(b)
    c = np.array(c)

    radians = np.arctan2(c[1] - b[1], c[0] - b[0]) - np.arctan2(a[1] - b[1], a[0] - b[0])
    angle = np.abs(radians * 180.0 / np.pi)

    if angle > 180.0:
        angle = 360 - angle

    return angle


cap = cv2.VideoCapture(0)

with mp_hands.Hands(min_detection_confidence=0.5, min_tracking_confidence=0.5) as hands, mp_pose.Pose(
        min_detection_confidence=0.5, min_tracking_confidence=0.5) as pose:
    right_hand_landmarks = None  # Variável para rastrear a mão direita dominante

    while cap.isOpened():
        ret, frame = cap.read()

        frame = cv2.cvtColor(cv2.flip(frame, 1), cv2.COLOR_BGR2RGB)
        frame.flags.writeable = False

        results = hands.process(frame)
        pose_results = pose.process(frame)

        frame.flags.writeable = True
        frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)

        # Verifica se há uma mão direita dominante no frame
        right_hand_present = False

        if results.multi_hand_landmarks:
            for hand_number, hand_landmarks in enumerate(results.multi_hand_landmarks):
                handedness = results.multi_handedness[hand_number].classification[0].label

                if handedness == "Right":
                    right_hand_landmarks = hand_landmarks
                    right_hand_present = True

        # Se a mão direita não estiver presente, limpe os dados anteriores
        if not right_hand_present:
            right_hand_landmarks = None

        if right_hand_landmarks is not None:
            mp_drawing.draw_landmarks(frame, right_hand_landmarks, mp_hands.HAND_CONNECTIONS)

            thumb = index = middle = ring = pinky = arm_state = hand_orientation_state = 0

            if right_hand_landmarks.landmark[mp_hands.HandLandmark.THUMB_MCP].x < right_hand_landmarks.landmark[
                mp_hands.HandLandmark.PINKY_MCP].x:
                hand_orientation = "forward"
                hand_orientation_state = 0
            else:
                hand_orientation = "backward"
                hand_orientation_state = 1

            if hand_orientation == "forward":
                if right_hand_landmarks.landmark[mp_hands.HandLandmark.THUMB_TIP].x < right_hand_landmarks.landmark[
                    mp_hands.HandLandmark.THUMB_IP].x:
                    thumb = 1
            else:
                if right_hand_landmarks.landmark[mp_hands.HandLandmark.THUMB_TIP].x > right_hand_landmarks.landmark[
                    mp_hands.HandLandmark.THUMB_IP].x:
                    thumb = 1

            if right_hand_landmarks.landmark[mp_hands.HandLandmark.INDEX_FINGER_TIP].y < right_hand_landmarks.landmark[
                mp_hands.HandLandmark.INDEX_FINGER_PIP].y:
                index = 1

            if right_hand_landmarks.landmark[mp_hands.HandLandmark.MIDDLE_FINGER_TIP].y < right_hand_landmarks.landmark[
                mp_hands.HandLandmark.MIDDLE_FINGER_PIP].y:
                middle = 1

            if right_hand_landmarks.landmark[mp_hands.HandLandmark.RING_FINGER_TIP].y < right_hand_landmarks.landmark[
                mp_hands.HandLandmark.RING_FINGER_PIP].y:
                ring = 1

            if right_hand_landmarks.landmark[mp_hands.HandLandmark.PINKY_TIP].y < right_hand_landmarks.landmark[
                mp_hands.HandLandmark.PINKY_PIP].y:
                pinky = 1

            if pose_results.pose_landmarks:
                elbow_angle = calculate_angle(
                    [pose_results.pose_landmarks.landmark[mp_pose.PoseLandmark.LEFT_SHOULDER].x,
                     pose_results.pose_landmarks.landmark[mp_pose.PoseLandmark.LEFT_SHOULDER].y],
                    [pose_results.pose_landmarks.landmark[mp_pose.PoseLandmark.LEFT_ELBOW].x,
                     pose_results.pose_landmarks.landmark[mp_pose.PoseLandmark.LEFT_ELBOW].y],
                    [pose_results.pose_landmarks.landmark[mp_pose.PoseLandmark.LEFT_WRIST].x,
                     pose_results.pose_landmarks.landmark[mp_pose.PoseLandmark.LEFT_WRIST].y]
                )
                elbow_angle = round(elbow_angle)

                if elbow_angle > 100:
                    arm_state = 2
                elif 70 <= elbow_angle <= 100:
                    arm_state = 1
                else:
                    arm_state = 0

                cv2.putText(frame, str(elbow_angle),
                            tuple(np.multiply(
                                [pose_results.pose_landmarks.landmark[mp_pose.PoseLandmark.LEFT_SHOULDER].x,
                                 pose_results.pose_landmarks.landmark[mp_pose.PoseLandmark.LEFT_SHOULDER].y],
                                [640, 480]).astype(int)),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2, cv2.LINE_AA)

            finger_string = f"${thumb}{index}{middle}{ring}{pinky}{arm_state}{hand_orientation_state}"
            ser.write(finger_string.encode())
            cv2.putText(frame, finger_string, (10, 50),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2, cv2.LINE_AA)

        cv2.imshow('MediaPipe Hands and Pose', frame)

        if cv2.waitKey(10) & 0xFF == ord('q'):
            break

ser.close()
cap.release()
cv2.destroyAllWindows()