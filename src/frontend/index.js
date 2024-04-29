window.addEventListener('load', () => {
    try {
        const websocket = new WebSocket(`ws://${window.location.hostname}/ws`);
        const percentage_slider_ = document.getElementById('percentage-slider');

        websocket.onopen = (event) => {
            percentage_slider_.addEventListener('change', () => { websocket.send(percentage_slider_.value); });
            console.log(`Connection established with ${window.location.hostname}`);
            console.log(event);
        };

        websocket.onclose = (event) => {
            console.log(`Connection with ${window.location.hostname} closed`);
            console.log(event);
        };

        websocket.onerror = (error) => {
            console.log("Websocket error");
            console.log(error);
        };

        websocket.onmessage = (event) => {
            const data = event.data;
            console.log(data);
            if (data > -1) {
                percentage_slider_.value = data;
            }
        };
    } catch (error) {
        console.log("Failed to connect to websocket")
        console.log(error)
    }
});

function motorMove(element) {
    const xhr = new XMLHttpRequest();
    xhr.open('GET', '/motor?position=' + element, true);
    xhr.send();
}

function motorStop(element) {
    const xhr = new XMLHttpRequest();
    xhr.open('GET', '/motor?stop=1', true);
    xhr.send();
}