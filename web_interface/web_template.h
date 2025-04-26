#pragma once
#include <string>

const std::string HTML_TEMPLATE = R"###(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SvaklaAI - Veridian Zenith</title>
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link href="https://fonts.googleapis.com/css2?family=Gamja+Flower&display=swap" rel="stylesheet">
    <style>
        body {
            font-family: "Gamja Flower", sans-serif;
            background-color: #111;
            color: #FFD700;
            margin: 0;
            padding: 0;
            text-align: center;
        }

        .header {
            background: rgba(0, 0, 0, 0.8);
            padding: 20px;
            border-radius: 20px;
            margin-bottom: 20px;
        }

        .header h1 {
            margin: 0;
            font-size: 2em;
            color: #FFD700;
        }

        .nav {
            list-style: none;
            padding: 0;
            margin: 20px 0;
            background: rgba(0, 0, 0, 0.8);
            padding: 20px;
            border-radius: 20px;
        }

        .nav a {
            color: gold;
            text-decoration: none;
            font-size: 1.2em;
            transition: 0.3s;
            margin: 0 15px;
            display: inline-block;
        }

        .nav a:hover {
            color: white;
            transform: scale(1.1);
        }

        .content {
            max-width: 1200px;
            margin: 0 auto;
            padding: 2rem;
        }

        .chat-container {
            padding: 50px;
            border-radius: 15px;
            background: linear-gradient(45deg, black, gold);
            margin: 30px auto;
            max-width: 90%;
        }

        .chat-messages {
            height: 300px;
            overflow-y: auto;
            margin-bottom: 1rem;
            padding: 1rem;
            background: rgba(0, 0, 0, 0.8);
            border-radius: 20px;
            text-align: left;
        }

        .chat-messages::-webkit-scrollbar {
            width: 8px;
        }

        .chat-messages::-webkit-scrollbar-track {
            background: rgba(0, 0, 0, 0.3);
            border-radius: 4px;
        }

        .chat-messages::-webkit-scrollbar-thumb {
            background: gold;
            border-radius: 4px;
        }

        .chat-input {
            display: flex;
            gap: 1rem;
            margin-top: 20px;
        }

        .chat-input input {
            flex: 1;
            padding: 0.5rem;
            border: none;
            border-radius: 20px;
            background: rgba(0, 0, 0, 0.8);
            color: #FFD700;
            font-family: "Gamja Flower", sans-serif;
            font-size: 1.1em;
        }

        .chat-input input:focus {
            outline: 2px solid gold;
        }

        .chat-input button {
            padding: 0.5rem 2rem;
            border: none;
            border-radius: 20px;
            background: rgba(0, 0, 0, 0.8);
            color: gold;
            cursor: pointer;
            transition: 0.3s;
            font-family: "Gamja Flower", sans-serif;
            font-size: 1.1em;
        }

        .chat-input button:hover {
            background: gold;
            color: black;
            transform: scale(1.05);
        }

        .status {
            text-align: right;
            margin-top: 0.5rem;
            color: gold;
            font-size: 0.9em;
        }

        .message {
            margin: 8px 0;
            padding: 8px 12px;
            border-radius: 10px;
            background: rgba(255, 215, 0, 0.1);
        }

        .message .sender {
            color: gold;
            font-weight: bold;
            margin-right: 8px;
        }

        .message .time {
            color: rgba(255, 215, 0, 0.6);
            font-size: 0.8em;
            float: right;
        }

        .system-message {
            background: rgba(0, 0, 0, 0.5);
            color: rgba(255, 215, 0, 0.7);
            font-style: italic;
        }

        .footer {
            background: rgba(0, 0, 0, 0.8);
            padding: 1rem;
            text-align: center;
            position: fixed;
            bottom: 0;
            width: 100%;
            color: gold;
        }

        .modal-overlay {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.95);
            display: flex;
            justify-content: center;
            align-items: center;
            z-index: 1000;
        }

        .startup-modal {
            background: linear-gradient(45deg, #111, #333);
            border: 2px solid gold;
            border-radius: 15px;
            padding: 30px;
            width: 90%;
            max-width: 600px;
            color: gold;
        }

        .startup-modal h2 {
            color: gold;
            text-align: center;
            margin-bottom: 30px;
            font-size: 2em;
        }

        .preferences-form {
            display: flex;
            flex-direction: column;
            gap: 20px;
        }

        .preference-group {
            display: flex;
            flex-direction: column;
            gap: 10px;
        }

        .preference-group label {
            color: gold;
            font-size: 1.1em;
        }

        .preference-group input[type="text"],
        .preference-group input[type="color"],
        .preference-group select {
            background: rgba(0, 0, 0, 0.8);
            border: 1px solid gold;
            border-radius: 10px;
            padding: 8px;
            color: gold;
            font-family: "Gamja Flower", sans-serif;
        }

        .preference-group input[type="checkbox"] {
            width: 20px;
            height: 20px;
            accent-color: gold;
        }

        .theme-preview {
            margin: 20px 0;
            padding: 15px;
            border-radius: 10px;
            background: rgba(0, 0, 0, 0.8);
        }

        .start-button {
            background: gold;
            color: black;
            border: none;
            border-radius: 25px;
            padding: 12px 30px;
            font-size: 1.2em;
            cursor: pointer;
            transition: 0.3s;
            font-family: "Gamja Flower", sans-serif;
            margin-top: 20px;
            width: 100%;
        }

        .start-button:hover {
            transform: scale(1.05);
            background: #ffd700;
        }
    </style>
    <script>
        let preferences = {
            username: localStorage.getItem('username') || '',
            theme: localStorage.getItem('theme') || 'dark-gold',
            fontSize: localStorage.getItem('fontSize') || '16',
            notifications: localStorage.getItem('notifications') === 'true',
            soundEffects: localStorage.getItem('soundEffects') === 'true',
            messageFormat: localStorage.getItem('messageFormat') || 'compact'
        };

        // Cache DOM elements
        let messageContainer;
        let inputField;
        window.addEventListener('DOMContentLoaded', () => {
            messageContainer = document.getElementById('messages');
            inputField = document.getElementById('messageInput');
        });

        function savePreferences() {
            preferences.username = document.getElementById('username').value;
            preferences.theme = document.getElementById('theme').value;
            preferences.fontSize = document.getElementById('fontSize').value;
            preferences.notifications = document.getElementById('notifications').checked;
            preferences.soundEffects = document.getElementById('soundEffects').checked;
            preferences.messageFormat = document.getElementById('messageFormat').value;

            Object.keys(preferences).forEach(key => {
                localStorage.setItem(key, preferences[key]);
            });

            applyPreferences();
            document.getElementById('startupModal').style.display = 'none';
            connectWebSocket();
        }

        function applyPreferences() {
            document.documentElement.style.setProperty('--font-size', preferences.fontSize + 'px');

            const themes = {
                'dark-gold': {
                    background: '#111',
                    text: '#FFD700',
                    accent: 'gold'
                },
                'dark-blue': {
                    background: '#111',
                    text: '#4a90e2',
                    accent: '#357abd'
                },
                'dark-green': {
                    background: '#111',
                    text: '#50C878',
                    accent: '#2E8B57'
                }
            };

            const theme = themes[preferences.theme];
            if (theme) {
                document.documentElement.style.setProperty('--primary-color', theme.background);
                document.documentElement.style.setProperty('--text-color', theme.text);
                document.documentElement.style.setProperty('--accent-color', theme.accent);
            }
        }

        let ws = null;
        const MAX_RECONNECT_ATTEMPTS = 5;
        let reconnectAttempts = 0;

        function connectWebSocket() {
            const protocol = window.location.protocol === 'https:' ? 'wss://' : 'ws://';
            const wsUrl = protocol + window.location.hostname + ':776';

            ws = new WebSocket(wsUrl);

            ws.onopen = () => {
                console.log('Connected to server');
                document.getElementById('status').textContent = 'Connected';
                reconnectAttempts = 0;
                if (preferences.username) {
                    registerUser();
                }
            };

            ws.onclose = () => {
                console.log('Disconnected from server');
                document.getElementById('status').textContent = 'Disconnected';
                if (reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
                    document.getElementById('status').textContent = `Reconnecting (attempt ${reconnectAttempts + 1}/${MAX_RECONNECT_ATTEMPTS})...`;
                    setTimeout(() => {
                        reconnectAttempts++;
                        connectWebSocket();
                    }, 1000 * Math.pow(2, reconnectAttempts));
                } else {
                    document.getElementById('status').textContent = 'Failed to reconnect';
                }
            };

            ws.onerror = (error) => {
                console.error('WebSocket error:', error);
                document.getElementById('status').textContent = 'Connection error';
            };

            ws.onmessage = (event) => {
                const msg = JSON.parse(event.data);
                displayMessage(msg);
            };
        }

        function registerUser() {
            if (ws && preferences.username) {
                ws.send(JSON.stringify({register: preferences.username}));
            }
        }

        function sendMessage() {
            const input = document.getElementById('message-input');
            if (ws && input.value) {
                ws.send(JSON.stringify({message: input.value}));
                input.value = '';
            }
        }

        function displayMessage(data) {
            const messages = document.getElementById('messages');

            const messageDiv = document.createElement('div');
            messageDiv.className = 'message ' + (data.type === 'system' ? 'system-message' : '');

            if (data.type === 'error') {
                messageDiv.style.color = '#ff6b6b';
            }

            let content = '';
            if (data.sender) {
                content += '<span class="sender">' + data.sender + ':</span>';
            }
            content += '<span class="content">' + data.content + '</span>';
            if (data.timestamp) {
                content += '<span class="time">' + data.timestamp + '</span>';
            }

            messageDiv.innerHTML = content;
            messages.appendChild(messageDiv);
            messages.scrollTop = messages.scrollHeight;
        }

        window.onload = function() {
            document.getElementById('username').value = preferences.username;
            document.getElementById('theme').value = preferences.theme;
            document.getElementById('fontSize').value = preferences.fontSize;
            document.getElementById('notifications').checked = preferences.notifications;
            document.getElementById('soundEffects').checked = preferences.soundEffects;
            document.getElementById('messageFormat').value = preferences.messageFormat;
        };
    </script>
</head>
<body>
    <div id="startupModal" class="modal-overlay">
        <div class="startup-modal">
            <h2>Welcome to SvaklaAI</h2>
            <div class="preferences-form">
                <div class="preference-group">
                    <label for="username">Username:</label>
                    <input type="text" id="username" placeholder="Enter your username">
                </div>
                <div class="preference-group">
                    <label for="theme">Theme:</label>
                    <select id="theme">
                        <option value="dark-gold">Dark Gold</option>
                        <option value="dark-blue">Dark Blue</option>
                        <option value="dark-green">Dark Green</option>
                    </select>
                </div>
                <div class="preference-group">
                    <label for="fontSize">Font Size:</label>
                    <input type="range" id="fontSize" min="12" max="24" value="16">
                    <div class="theme-preview" id="fontPreview">Preview Text</div>
                </div>
                <div class="preference-group">
                    <label>
                        <input type="checkbox" id="notifications">
                        Enable Desktop Notifications
                    </label>
                </div>
                <div class="preference-group">
                    <label>
                        <input type="checkbox" id="soundEffects">
                        Enable Sound Effects
                    </label>
                </div>
                <div class="preference-group">
                    <label for="messageFormat">Message Format:</label>
                    <select id="messageFormat">
                        <option value="compact">Compact</option>
                        <option value="expanded">Expanded</option>
                        <option value="minimal">Minimal</option>
                    </select>
                </div>
                <button class="start-button" onclick="savePreferences()">Start Chat</button>
            </div>
        </div>
    </div>

    <div class="header">
        <h1>Veridian Zenith - SvaklaAI</h1>
    </div>

    <div class="nav">
        <a href="#chat">Chat</a>
        <a href="#projects">Projects</a>
        <a href="#minecraft">Minecraft</a>
        <a href="#downloads">Downloads</a>
    </div>

    <div class="content">
        <div class="chat-container">
            <div class="chat-messages" id="messages"></div>
            <div class="chat-input">
                <input type="text" id="message-input" placeholder="Type your message..."
                       onkeypress="if(event.key === 'Enter') sendMessage()">
                <button onclick="sendMessage()">Send</button>
            </div>
            <div class="status">
                Status: <span id="status">Connecting...</span>
            </div>
        </div>
    </div>

    <div class="footer">
        &copy; 2025 Veridian Zenith. All rights reserved.
    </div>
</body>
</html>
)###";
