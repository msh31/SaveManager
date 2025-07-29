// Platform configuration with proper SVG icons
const PLATFORMS = {
    Steam: {
        color: '#171a21',
        icon: `<svg viewBox="0 0 24 24" fill="white">
                <path d="M11.979 0C5.678 0 .511 4.86.022 11.037l6.432 2.658c.545-.371 1.203-.59 1.912-.59.063 0 .125.004.188.006l2.861-4.142V8.91c0-2.495 2.028-4.524 4.524-4.524 2.494 0 4.524 2.031 4.524 4.527s-2.03 4.525-4.524 4.525h-.105l-4.076 2.911c0 .052.004.105.004.159 0 1.875-1.515 3.396-3.39 3.396-1.635 0-3.016-1.173-3.331-2.727L.436 15.27C1.862 20.307 6.486 24 11.979 24c6.627 0 11.999-5.373 11.999-12S18.605 0 11.979 0zM7.54 18.21l-1.473-.61c.262.543.714.999 1.314 1.25 1.297.539 2.793-.076 3.332-1.375.263-.63.264-1.319.005-1.949s-.75-1.121-1.377-1.383c-.624-.26-1.29-.249-1.878-.03l1.523.63c.956.4 1.409 1.5 1.009 2.455-.397.957-1.497 1.41-2.454 1.012H7.54zm11.415-9.303c0-1.662-1.353-3.015-3.015-3.015-1.665 0-3.015 1.353-3.015 3.015 0 1.665 1.35 3.015 3.015 3.015 1.663 0 3.015-1.35 3.015-3.015zm-5.273-.005c0-1.252 1.013-2.266 2.265-2.266 1.249 0 2.266 1.014 2.266 2.266 0 1.251-1.017 2.265-2.266 2.265-1.253 0-2.265-1.014-2.265-2.265z"/>
               </svg>`
    },
    Ubisoft: {
        color: '#0085ff',
        icon: `<svg fill="white" width="800px" height="800px" viewBox="0 0 32 32" xmlns="http://www.w3.org/2000/svg">
                  <path d="M31.416 15.984c-0.348-16.391-22.145-22.505-30.541-7.14 0.376 0.276 0.88 0.635 1.256 0.896-0.62 1.296-1.057 2.676-1.303 4.093-0.161 0.912-0.244 1.833-0.244 2.76 0 8.5 6.911 15.407 15.421 15.407 8.516 0 15.411-6.896 15.411-15.407zM4.385 18.729c-0.203 1.667-0.073 2.183-0.073 2.385l-0.375 0.131c-0.14-0.272-0.489-1.24-0.651-2.543-0.407-4.957 2.979-9.421 8.14-10.265 4.724-0.692 9.251 2.245 10.303 6.349l-0.375 0.131c-0.115-0.115-0.303-0.448-1.027-1.172-5.708-5.709-14.672-3.095-15.943 4.989zM19.057 21.505c-0.787 1.135-2.079 1.812-3.453 1.807-2.328 0.005-4.213-1.88-4.208-4.208 0.005-2.197 1.708-4.025 3.901-4.187 1.359-0.057 2.629 0.676 3.224 1.864 0.651 1.301 0.411 2.88-0.604 3.927 0.389 0.276 0.765 0.537 1.14 0.797zM27.828 21.667c-2.224 5.041-6.807 7.688-11.692 7.615-9.381-0.464-12.109-11.287-5.839-15.188l0.276 0.271c-0.104 0.147-0.48 0.439-1.057 1.579-0.677 1.385-0.896 2.776-0.808 3.641 0.489 7.561 11.089 9.109 14.729 1.619 4.641-10.244-7.677-20.667-18.604-12.703l-0.245-0.245c2.876-4.509 8.5-6.52 13.86-5.176 8.197 2.067 12.604 10.609 9.38 18.588z"/>
               </svg>`
    }
};

let gameData = {};
let groupedGamesData = {};
let gameIdMappings = {};

fetch('https://raw.githubusercontent.com/msh31/Ubisoft-Game-Ids/refs/heads/master/gameids.json')
    .then(response => response.json())
    .then(data => {
        for (const franchise in data) {
            for (const gameId in data[franchise]) {
                gameIdMappings[gameId] = data[franchise][gameId];
            }
        }
    })
    .catch(error => console.error('Failed to load game ID mappings:', error));

function cleanGameName(gameName) {
    let cleaned = gameName.replace(/\s*\([^)]*\)\s*/g, ' ').trim(); //country tags
    cleaned = cleaned.replace(/\s*-?\s*(Steam|Uplay|Ubisoft)\s*$/i, '').trim();//platform tags
    cleaned = cleaned.replace(/\s+/g, ' ');

    return cleaned;
}

function detectPlatform(key, gameData) {
    const gameName = gameData.gameName;
    
    if (gameName.toLowerCase().includes('(steam)')) {
        return {
            name: 'Steam',
            icon: PLATFORMS.Steam.icon,
            color: PLATFORMS.Steam.color
        };
    }
    
    return {
        name: 'Ubisoft',
        icon: PLATFORMS.Ubisoft.icon,
        color: PLATFORMS.Ubisoft.color
    };
}

function groupGamesByName(data) {
    const grouped = {};

    for (const key in data) {
        const gameInfo = data[key];
        const platform = detectPlatform(key, gameInfo);
        const cleanName = cleanGameName(gameInfo.gameName);

        if (!grouped[cleanName]) {
            grouped[cleanName] = {};
        }

        grouped[cleanName][platform.name] = {
            key: key,
            gameData: gameInfo,
            platform: platform
        };
    }

    return grouped;
}

function createGameCard(cleanName, platforms) {
    const platformKeys = Object.keys(platforms);
    const firstPlatform = platforms[platformKeys[0]];

    let html = `
        <div class="game-card" data-game-name="${cleanName}">
            <div class="game-artwork ${firstPlatform.gameData.artworkUrl ? '' : 'no-artwork'}" 
                 ${firstPlatform.gameData.artworkUrl ? `style="background-image: url('${firstPlatform.gameData.artworkUrl}')"` : ''}>
                <div class="game-overlay">
                    <div class="game-title-row">
                        <h3>${cleanName}</h3>
                        <div class="platform-icons">
    `;
    
    for (let i = 0; i < platformKeys.length; i++) {
        const platformName = platformKeys[i];
        const platform = platforms[platformName].platform;
        const isActive = i === 0;

        html += `
            <span class="platform-icon ${isActive ? 'active' : ''}" 
                  data-platform="${platformName}"
                  title="${platform.name}"
                  style="background-color: ${platform.color}"
                  onclick="togglePlatform('${cleanName.replace(/'/g, "\\'")}', '${platformName}', this)">
                ${platform.icon}
            </span>
        `;
    }

    html += `
                        </div>
                    </div>
                </div>
            </div>
            <div class="save-list">
    `;
    
    html += createSavesList(firstPlatform);
    html += '</div></div>';
    return html;
}

function createSavesList(platformData) {
    const saves = platformData.gameData.saves;
    let html = '';

    for (let i = 0; i < saves.length; i++) {
        const save = saves[i];
        const size = (save.fileSizeBytes / 1024).toFixed(1);
        const date = new Date(save.lastModified).toLocaleDateString();
        const displayName = (save.displayName && save.displayName !== 'CUSTOM_NAME_NOT_SET') ? save.displayName : save.fileName;

        html += `
            <div class="save-item" data-game="${platformData.key}" data-save="${i}">
                <div class="save-icon">📁</div>
                <div class="save-details">
                    <div class="save-name">${displayName}</div>
                    <div class="save-meta">${size}KB • ${date}</div>
                </div>
                <div class="save-actions">
                    <button class="action-btn" onclick="deleteSave('${platformData.key}', ${i})" title="Delete">
                        🗑️
                    </button>
                    <button class="action-btn" onclick="selectSave('${platformData.key}', ${i}, this)" title="Select">
                        ☑️
                    </button>
                </div>
            </div>
        `;
    }

    return html;
}

async function loadSaves() {
    const content = document.getElementById('content');
    content.innerHTML = '<div class="loading"><div class="spinner"></div><p>Loading save games...</p></div>';

    try {
        const response = await fetch('/api/saves');
        const data = await response.json();

        if (data.error) {
            content.innerHTML = `<div class="loading"><p>Error: ${data.error}</p></div>`;
            return;
        }

        gameData = data;
        groupedGamesData = groupGamesByName(data);

        let totalSaves = 0;
        let html = '';

        for (const cleanName in groupedGamesData) {
            const platforms = groupedGamesData[cleanName];
            let allSaves = 0;

            for (const platformName in platforms) {
                allSaves += platforms[platformName].gameData.saves.length;
            }

            totalSaves += allSaves;
            html += createGameCard(cleanName, platforms);
        }

        if (html) {
            content.innerHTML = html;
        } else {
            content.innerHTML = '<div class="loading"><p>No save games found.</p></div>';
        }

        const gameCount = Object.keys(groupedGamesData).length;
        document.getElementById('totalSaves').textContent = `${totalSaves} saves across ${gameCount} games`;
        
        addImageErrorHandling();
    } catch (error) {
        content.innerHTML = `<div class="loading"><p>Error loading data: ${error.message}</p></div>`;
    }
}

function addImageErrorHandling() {
    const artworkElements = document.querySelectorAll('.game-artwork');

    for (const element of artworkElements) {
        const backgroundImage = element.style.backgroundImage;
        if (backgroundImage && backgroundImage !== 'none') {
            const img = new Image();
            img.onload = function() {
                
            };
            img.onerror = function() {
                element.style.backgroundImage = 'none';
                element.classList.add('no-artwork');
            };
            
            const urlMatch = backgroundImage.match(/url\(['"]?(.*?)['"]?\)/);
            if (urlMatch) {
                img.src = urlMatch[1];
            }
        }
    }
}

function togglePlatform(gameName, platformName, iconElement) {
    const card = iconElement.closest('.game-card');
    const allIcons = card.querySelectorAll('.platform-icon');

    for (const icon of allIcons) {
        icon.classList.remove('active');
    }
    iconElement.classList.add('active');

    const savesList = card.querySelector('.save-list');
    const platformData = groupedGamesData[gameName][platformName];
    savesList.innerHTML = createSavesList(platformData);

    const artwork = card.querySelector('.game-artwork');
    
    if (platformData.gameData.artworkUrl) {
        artwork.style.backgroundImage = `url('${platformData.gameData.artworkUrl}')`;
        artwork.classList.remove('no-artwork');
    } else {
        artwork.style.backgroundImage = 'none';
        artwork.classList.add('no-artwork');
    }
}

function deleteSave(gameKey, saveIndex) {
    if (confirm('Delete this save file?')) {
        console.log(`Deleting save ${saveIndex} from game ${gameKey}`);
        alert('Delete functionality coming soon!');
    }
}

function selectSave(gameKey, saveIndex, button) {
    button.classList.toggle('selected');
    console.log(`Toggled save ${saveIndex} from game ${gameKey}`);
}

async function backupSaves() {
    alert('Backup functionality coming soon!');
}

async function restoreSaves() {
    alert('Restore functionality coming soon!');
}

async function syncSaves() {
    alert('Sync functionality coming soon!');
}

async function deleteSelected() {
    const selected = document.querySelectorAll('.action-btn.selected');
    if (selected.length === 0) {
        alert('Please select saves to delete');
        return;
    }
    if (confirm(`Delete ${selected.length} selected saves?`)) {
        alert('Delete functionality coming soon!');
    }
}

loadSaves();
setInterval(loadSaves, 30000);