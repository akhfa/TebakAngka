#include <iostream>
#include <curl/curl.h>
#include <boost/algorithm/string.hpp>
#include "crow_all.h"

std::string account_sid = "";
std::string auth_token = "";

std::map<std::string, int> state;

bool send_message(
        const std::string &to_number,
        const std::string &from_number,
        const std::string &message_body,
        std::string &response) {
    std::stringstream response_stream;

    if (message_body.length() > 1600) {
        response_stream << "Message body must have 1600 or fewer"
                        << " characters. Cannot send message with "
                        << message_body.length() << " characters.";
        response = response_stream.str();
        return false;
    }

    CURL *curl;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    std::stringstream url;
    std::string url_string;
    url << "https://api.twilio.com/2010-04-01/Accounts/" << account_sid << "/Messages";
    url_string = url.str();

    std::stringstream parameters;
    std::string parameter_string;
    parameters << "To=" << to_number << "&From=" << from_number << "&Body=" << message_body;
    parameter_string = parameters.str();

    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_URL, url_string.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameter_string.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, account_sid.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, auth_token.c_str());

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    long http_code = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);

    // Check for curl errors and Twilio failure status codes.
    if (res != CURLE_OK) {
        response = curl_easy_strerror(res);
        return false;
    } else if (http_code != 200 && http_code != 201) {
        response = response_stream.str();
        return false;
    } else {
        response = response_stream.str();
        return true;
    }
}

int generate_number() {
    int bitmask = 0;
    int number = 0;
    for (int i = 0; i < 4; i++) {
        int digit;
        do {
            digit = rand() % 10;
        } while ((bitmask & (1 << digit)) > 0);
        bitmask |= (1 << digit);
        number *= 10;
        number += digit;
    }
    return number;
}

void process(std::map<std::string, std::string> &mp) {
    int curstate = state[mp["From"]];
    std::string response;
    std::string message;
    if (mp["Body"] == "%2Fplay") {
        if (curstate == 0) {
            message = "Guess a 4-digit number.\n"
                      "All of the digits are different.\n"
                      "Type /quit to stop playing.\n"
                      "Type 4-digit number (your guess) to keep playing.";
            state[mp["From"]] = generate_number();
            send_message(mp["From"], mp["To"], message, response);
        }
    } else if (mp["Body"] == "%2Fquit") {
        if (curstate != 0) {
            message = "You quit the game.\n"
                      "The answer is: " + std::to_string(curstate);
            state[mp["From"]] = 0;
            send_message(mp["From"], mp["To"], message, response);
        }
    } else {
        if (curstate == 0) {
            message = "Type /play to start playing.";
            send_message(mp["From"], mp["To"], message, response);
        } else {
            std::string guess = mp["Body"];
            if (guess.size() != 4) {
                message = "Guess must be 4 digits.";
                send_message(mp["From"], mp["To"], message, response);
                return;
            }
            int state_digit[4];
            int state_bitmask = 0;
            int cur = curstate;
            for (int i=0; i<4; i++) {
                state_digit[3 - i] = cur % 10;
                state_bitmask |= (1 << state_digit[3 - i]);
                cur /= 10;
            }
            int a = 0;
            int b = 0;
            int bitmask = 0;
            int guess_value = 0;
            for (int i=0; i<4; i++) {
                if (guess[i] < '0' || guess[i] > '9') {
                    message = "Guess must be number only.";
                    send_message(mp["From"], mp["To"], message, response);
                    return;
                }
                int x = (int)(guess[i] - '0');
                if ((bitmask & (1 << x)) > 0) {
                    message = "Guess must be unique digits.";
                    send_message(mp["From"], mp["To"], message, response);
                    return;
                }
                guess_value *= 10;
                guess_value += x;
                bitmask |= (1 << x);
                if ((state_bitmask & (1 << x)) > 0) {
                    b++;
                }
                if (state_digit[i] == x) {
                    a++;
                }
            }
            b -= a;
            if (a == 4) {
                message = "Congratulation, your guess is right!";
                state[mp["From"]] = 0;
                send_message(mp["From"], mp["To"], message, response);
            } else {
                message = std::to_string(a) + " digit(s) is/are in right position(s).\n" +
                          std::to_string(b) + " digit(s) is/are right, but in wrong position(s).";
                send_message(mp["From"], mp["To"], message, response);
            }
        }
    }
}

int main() {
    srand(time(NULL));
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "Hello cpp";
    });

    CROW_ROUTE(app, "/message")
            .methods("POST"_method)
                    ([](const crow::request &req) {
                        std::vector<std::string> result;
                        boost::split(result, req.body, boost::is_any_of("&"));
                        std::map<std::string, std::string> mp;
                        for (int i = 0; i < result.size(); i++) {
                            std::vector<std::string> res2;
                            boost::split(res2, result[i], boost::is_any_of("="));
                            mp[res2[0]] = res2[1];
                        }
                        process(mp);
                        std::ostringstream os;
                        os << "OK";
                        return crow::response{os.str()};
                    });

    app.port(18080).multithreaded().run();
    return 0;
}
