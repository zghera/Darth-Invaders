#include "stm32f0xx.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "lcd.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Peripheral Initialization and Picture Helpers
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//============================================================================
// Read and Debounce keypad functions adapted from Lab 7
//============================================================================
void set_row(int rowNum) {
    GPIOB->BSRR = 0xf0000 | (1 << (rowNum - 1));
}

int get_cols() {
    return (GPIOB->IDR >> 4) & 0xf;
}

const char buttonChar[] = { '*', '0', '#', 'D', '\0' };

int get_key() {
    int cols = get_cols();

    int pressedIdx = 4; // Default to null byte for defense
    for (int i = 0; i < 4; i++) {
        if ((cols & (1 << i)) != 0) {
            pressedIdx = i;
            break;
        }
    }
    return buttonChar[pressedIdx];
}

void setup_portb() {
    // Enable Port B.
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    // Set PB0-PB3 for output.
    GPIOB->MODER &= ~0xff;
    GPIOB->MODER |= 0x55;
    // Set PB4-PB7 for input and enable pull-down resistors.
    GPIOB->MODER &= ~0xff00;
    GPIOB->PUPDR &= ~0xff00;
    GPIOB->PUPDR |= 0xaa00;
    // Turn on the output for the lower row.
    set_row(4);
}

//============================================================================
// OLED LCD and TFT LCD (SPI) Functionality from Lab 8
//============================================================================
void setup_spi2(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->MODER &= ~(GPIO_MODER_MODER12 | GPIO_MODER_MODER13
            | GPIO_MODER_MODER15);
    GPIOB->MODER |= GPIO_MODER_MODER12_1 | GPIO_MODER_MODER13_1
            | GPIO_MODER_MODER15_1;
    GPIOB->AFR[1] &= ~GPIO_AFRH_AFR12 | ~GPIO_AFRH_AFR13 | ~GPIO_AFRH_AFR15;

    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
    // Disable SPI1 to configure the channel
    SPI2->CR1 &= ~SPI_CR1_SPE;
    // Set the baud rate as low as possible
    SPI2->CR1 |= SPI_CR1_BR;
    // Configure the SPI channel to be in master mode and set transfer direction
    // to transmit-only using half-duplex communication mode (single
    // bi-directional data line)
    SPI2->CR1 |= SPI_CR1_MSTR | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;
    // Enable output of NSS and use NSSP to strobe NSS automatically as well as
    // configure the interface for a 10-bit word size.
    SPI2->CR2 = SPI_CR2_SSOE | SPI_CR2_NSSP | SPI_CR2_DS_3 | SPI_CR2_DS_0;
    SPI2->CR1 |= SPI_CR1_SPE;
}

void spi_cmd(int cmd) {
    while ((SPI2->SR & SPI_SR_TXE) == 0)
        ; // wait for the transmit buffer to be empty
    SPI2->DR = cmd;
}

void spi_data(int data) {
    while ((SPI2->SR & SPI_SR_TXE) == 0)
        ; // wait for the transmit buffer to be empty
    SPI2->DR = 0x200 | data;
}

void spi_init_oled(void) {
    nano_wait(1000000); // wait 1 ms for the display to power up and stabilize.
    spi_cmd(0x38); // set for 8-bit operation
    spi_cmd(0x08); // turn display off
    spi_cmd(0x01); // clear display
    nano_wait(2000000); // wait 2 ms for the display to clear.
    spi_cmd(0x06); // set the display to scroll
    spi_cmd(0x02); // move the cursor to the home position
    spi_cmd(0x0c); // turn the display on
}

void spi_display1(char* row1Msg) {
    spi_cmd(0x02); // move the cursor to the home position
    for (int i = 0; row1Msg[i] != 0; i++) {
        spi_data(row1Msg[i]);
    }
}

void spi_display2(const char* row2Msg) {
    spi_cmd(0xc0); // move the cursor to the lower row (offset 0x40)
    for (int i = 0; row2Msg[i] != 0; i++) {
        spi_data(row2Msg[i]);
    }
}

void setup_spi1() {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    // Configure PA4, PA5, and PA7 for alternate function 0 and PA2 and PA3 to
    // be outputs
    GPIOA->MODER &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3 | GPIO_MODER_MODER4
            | GPIO_MODER_MODER5 | GPIO_MODER_MODER7);
    GPIOA->MODER |= GPIO_MODER_MODER2_0 | GPIO_MODER_MODER3_0
            | GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1 | GPIO_MODER_MODER7_1;
    GPIOA->AFR[0] &= ~GPIO_AFRL_AFR4 | ~GPIO_AFRL_AFR5 | ~GPIO_AFRL_AFR7;

    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    // Disable SPI1 to configure the channel
    SPI1->CR1 &= ~SPI_CR1_SPE;
    // Set the baud rate as high as possible
    SPI1->CR1 &= ~SPI_CR1_BR;
    // Configure the SPI channel to be in master mode and set transfer direction
    // to transmit-only using half-duplex communication mode (single
    // bi-directional data line)
    SPI1->CR1 |= SPI_CR1_MSTR | SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;
    // Enable output of NSS and use NSSP to strobe NSS automatically as well as
    // configure the interface for a 8-bit word size (default).
    SPI1->CR2 = SPI_CR2_SSOE | SPI_CR2_NSSP;
    SPI1->CR1 |= SPI_CR1_SPE;
}

//============================================================================
// setup_tim2()
// Configure timer 2 to invoke 24 million times per second.
// Parameters: none.
// Return value: void.
//============================================================================
void setup_tim2() {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 1 - 1;
    TIM2->ARR = 0xffff - 1;
    TIM2->CR1 |= TIM_CR1_CEN;
}

//============================================================================
// setup_tim6()
// Configure timer 6 to invoke 100 times per second.
// Parameters: none.
// Return value: void.
//============================================================================
void setup_tim6() {
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6->PSC = 48000 - 1;
    TIM6->ARR = 10 - 1;         // 100 Hz = 48 MHz / 48000 / 10
    TIM6->DIER |= TIM_DIER_UIE;
    TIM6->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] = 1 << TIM6_DAC_IRQn;
}

//============================================================================
// setup_tim7()
// Configure timer 7 to invoke 2 times per second.
// Parameters: none.
// Return value: void.
//============================================================================
void setup_tim7() {
    RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7->PSC = 48000 - 1;
    TIM7->ARR = 500 - 1;         // 2 Hz = 48 MHz / 48000 / 500
    TIM7->DIER |= TIM_DIER_UIE;
    TIM7->CR1 |= TIM_CR1_ARPE;
    TIM7->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] = 1 << TIM7_IRQn;
}

//============================================================================
// pic_subset()
// Copy a subset of a large source picture into a smaller destination.
// sx,sy are the offset into the source picture.
//============================================================================
void pic_subset(Picture *dst, const Picture *src, int sx, int sy) {
    int dw = dst->width;
    int dh = dst->height;
    if (dw + sx > src->width)
        assert(0);
    if (dh + sy > src->height)
        assert(0);
    for (int y = 0; y < dh; y++)
        for (int x = 0; x < dw; x++)
            dst->pix2[dw * y + x] = src->pix2[src->width * (y + sy) + x + sx];
}

//============================================================================
// pic_overlay()
// Overlay a picture onto a destination picture.
// xoffset,yoffset are the offset into the destination picture that the
// source picture is placed.
// Any pixel in the source that is the 'transparent' color will not be
// copied.  This defines a color in the source that can be used as a
// transparent color.
//============================================================================
void pic_overlay(Picture *dst, int xoffset, int yoffset, const Picture *src,
        int transparent) {
    for (int y = 0; y < src->height; y++) {
        int dy = y + yoffset;
        if (dy < 0 || dy >= dst->height)
            continue;
        for (int x = 0; x < src->width; x++) {
            int dx = x + xoffset;
            if (dx < 0 || dx >= dst->width)
                continue;
            unsigned short int p = src->pix2[y * src->width + x];
            if (p != transparent)
                dst->pix2[dy * dst->width + dx] = p;
        }
    }
}

//============================================================================
// pic_overlay()
// Returns a heap allocated Picture with dimensions 'w'x'h' pixels.
//============================================================================
Picture* get_new_picture(unsigned int w, unsigned int h) {
    size_t numPicElements = (w * h) / 6 + 2;
    Picture* p = malloc(numPicElements * sizeof(*p));
    for (int i = 0; i < numPicElements; i++) {
        p[i] = (Picture) {.width = w, .height = h, .bytes_per_pixel = 2};
    }
    return p;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Main Program
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
extern const Picture backgroundImg; // 240x320 Background image
//extern const Picture gameOverImg; // 240x320 Game over screen image
extern const Picture shieldImg;     // 45x5 Blue rectangle (shields)
extern const Picture falconImg;     // 25x33 Millenium Falcon with white boundaries
extern const Picture tieImg;        // 25x23 TIE fighter with white boundaries
extern const Picture blackRectImg;  // 25x23 Black Rectangle to draw over
                                    //  the TIE fighter once destroyed
extern const Picture tieExplosionImg; // 25x23 Explosion to replace tie fighter
                                      // when it is shot.
extern const Picture falconExplosionImg; // 25x33 Explosion to replace Millenium
                                         // when it is shot.
extern const Picture vaderImg; // 120x120 Picture of Darth Vader

// The game will continue until one of three conditions are met
// 1. The player (Millenium Falcon) shoots down all enemy (TIE) fighters
// 2. The player is shot down by a TIE fighter
// 3. The TIE fighter's reach the player's position
bool gameIsOver;
size_t score;

const size_t border = 10;
size_t xmin; // Farthest to the left the center any ship can go
size_t xmax; // Farthest to the right the center any ship can go
size_t ymin; // Farthest to the top the center any ship can go
size_t ymax; // Farthest to the bottom the center any ship can go

#define LASER_LEN 12
typedef struct {
    size_t x;
    size_t yI;
    size_t yF;
    int pixelDy;
    uint16_t color;
} Laser;
static const Laser resetLaser;
Laser falconShot;
Laser tieShots[3];

typedef enum {
    ACTIVE = 1,
    EXPLODING,
    DESTROYED
} ShipState;

typedef struct {
    const Picture* image;
    size_t width;
    size_t height;
    size_t pixelDx;
    size_t pixelDy;
    size_t xCenter;
    size_t yCenter;
    size_t scoreGain;
    ShipState state;
} Ship;
Ship falcon;

#define NUM_ROWS_OF_TIES 3
#define NUM_TIES_PER_ROW 6
Ship ties[NUM_ROWS_OF_TIES][NUM_TIES_PER_ROW];
size_t numRemainingTies;
bool tiesMovingRight;

const size_t shieldsWidth = 45;
const size_t shieldsHeight = 5;
size_t shieldsCenterX[2];
size_t shieldsCenterY;

//============================================================================
// update_score()
//============================================================================
void update_score(size_t scoreGain) {
    score += scoreGain;
    char dispBuffer[20];
    sprintf(dispBuffer, "Score: %04d", score);
    spi_display1("                ");
    spi_display1("Nice shot!");
    spi_display2(dispBuffer);
}

//============================================================================
// draw_ship()
// Create a temporary object two pixels wider than the paddle.
// Copy the background into it.
// Overlay the paddle image into the center.
// Draw the result.
//
// As long as the paddle moves no more than one pixel to the left or right
// it will clear the previous parts of the paddle from the screen.
//============================================================================
void draw_ship(Ship* ship) {
    Picture* shipCopy = get_new_picture(ship->width + 2 * ship->pixelDx,
                                        ship->height + ship->pixelDy);
    size_t shipCopyUpperLeftX = ship->xCenter - shipCopy->width / 2;
    size_t shipCopyUpperLeftY = ship->yCenter - ship->height / 2 - ship->pixelDy;
    pic_subset(shipCopy, &backgroundImg, shipCopyUpperLeftX, shipCopyUpperLeftY);
    pic_overlay(shipCopy, ship->pixelDx, ship->pixelDy, ship->image, 0xffff);
    LCD_DrawPicture(shipCopyUpperLeftX, shipCopyUpperLeftY, shipCopy);
    free(shipCopy);
}

//============================================================================
// move_falcon()
// Re-renders the player ship (the Millenium Falcon ) 1 pixel to the left or
// right if the keyboard input is '*' or '0', respectively.
//============================================================================
void move_falcon(char input) {
    if (input == '*' &&
            falcon.xCenter - falconImg.width / 2 > border ) {
        falcon.xCenter -= 1;
    } else if (input == '0' && falcon.xCenter + falconImg.width / 2 <
                     backgroundImg.width - border) {
        falcon.xCenter += 1;
    } else {
        return;
    }

    draw_ship(&falcon);
}

//============================================================================
// get_rightmost_tie_center_x()
//============================================================================
size_t get_rightmost_tie_center_x(void) {
    size_t rightmostTieCenterX = xmin;
    for (size_t row = 0; row < NUM_ROWS_OF_TIES; row++) {
        for (size_t col = 0; col < NUM_TIES_PER_ROW; col++) {
            Ship curTie = ties[row][col];
            if (curTie.state == ACTIVE) {
                if (curTie.xCenter > rightmostTieCenterX) {
                    rightmostTieCenterX = curTie.xCenter;
                }
            }
        }
    }
    return rightmostTieCenterX;
}

//============================================================================
// get_rightmost_tie_center_x()
//============================================================================
size_t get_leftmost_tie_center_x(void) {
    size_t leftmostTieCenterX = xmax;
    for (size_t row = 0; row < NUM_ROWS_OF_TIES; row++) {
        for (size_t col = 0; col < NUM_TIES_PER_ROW; col++) {
            Ship curTie = ties[row][col];
            if (curTie.state == ACTIVE) {
                if (curTie.xCenter < leftmostTieCenterX) {
                    leftmostTieCenterX = curTie.xCenter;
                }
            }
        }
    }
    return leftmostTieCenterX;
}

//============================================================================
// get_edge_tie_center_x()
// Returns the x position of the center of rightmost/leftmost tie fighter
// based on 'tiesMovingRight'.
//============================================================================
size_t get_edge_tie_center_x(void) {
    return (tiesMovingRight) ? get_rightmost_tie_center_x() :
                               get_leftmost_tie_center_x();
}

//============================================================================
// get_edge_tie_center_x()
// Returns the y position of the center of bottom-most tie fighter.
//============================================================================
size_t get_bottom_tie_center_y(void) {
    for (int row = NUM_ROWS_OF_TIES - 1; row >= 0; row--) {
        for (int col = 0; col < NUM_TIES_PER_ROW; col++) {
            Ship curTie = ties[row][col];
            if (curTie.state == ACTIVE) {
                return curTie.yCenter;
            }
        }
    }
    return 0;
}

//============================================================================
// move_ties()
// Re-renders the enemy ships (TIE fighters) as they move across and down the
// screen based on the flag 'tiesMovingRight'. The TIEs will move right until
// the rightmost tie hits the right border. At this point, all ties will move
// down the screen and start moving left. This will continue until the game is
// over.
//============================================================================
void move_ties(void) {
    int tiesCenterDx = tiesMovingRight ? (ties[0][0]).pixelDx
                                       : -((ties[0][0]).pixelDx);
    for (int row = 0; row < NUM_ROWS_OF_TIES; row++) {
        for (int col = NUM_TIES_PER_ROW - 1; col >= 0; col--) {
            Ship* curTie = &(ties[row][col]);
            curTie->xCenter += tiesCenterDx;
            if (curTie->state == ACTIVE) {
                draw_ship(curTie);
            } else if (curTie->state == EXPLODING) {
                curTie->image = &blackRectImg;
                curTie->state = DESTROYED;
                draw_ship(curTie);
            }
        }
    }

    size_t edgeTieCenterX = get_edge_tie_center_x();
    // If we are at the edge of the screen, shift down
    if ((tiesMovingRight && edgeTieCenterX >= xmax) ||
        (!tiesMovingRight && edgeTieCenterX <= xmin)) {
        // If the next shift down would cause overlap between TIE and the
        // shields, the game is over.
        size_t bottomTieCenterY = get_bottom_tie_center_y();
        if (bottomTieCenterY >= shieldsCenterY - (ties[0][0]).height) {
           gameIsOver = true;
           return;
        }

        for (int row = NUM_ROWS_OF_TIES - 1; row >= 0; row--) {
            for (int col = NUM_TIES_PER_ROW - 1; col >= 0; col--) {
                Ship* curTie = &(ties[row][col]);
                curTie->yCenter += (curTie->height) * 3 / 4;
                if (curTie->state == ACTIVE) {
                    // Need to set dY to erase top half of current ship
                    curTie->pixelDy = (curTie->height) * 3 / 4;
                    draw_ship(curTie);
                    curTie->pixelDy = 0;
                }
            }
        }
        tiesMovingRight = !tiesMovingRight;
    }
}

//============================================================================
// move_laser()
//============================================================================
void move_laser(Laser* laser) {
    size_t x = laser->x;
    size_t initY1 = laser->yI;
    laser->yI += laser->pixelDy;
    laser->yF += laser->pixelDy;
    // Erase old "tail" of laser
    LCD_DrawLine(x, initY1, x, laser->yI, BLACK);
    // Draw new "head" of laser
    LCD_DrawLine(x, laser->yI, x, laser->yF, laser->color);
}

//============================================================================
// erase_recently_shot_tie_lasers()
//============================================================================
void erase_recently_shot_tie_lasers(void) {
    size_t bottomTieCenterY = get_bottom_tie_center_y();
    for (int i = 0; i < sizeof(tieShots)/sizeof(tieShots[0]); i++) {
        Laser* curLaser = &(tieShots[i]);
        if (curLaser->yI <= bottomTieCenterY + tieImg.height * 3 / 2) {
            LCD_DrawLine(curLaser->x, curLaser->yI, curLaser->x,
                            curLaser->yF, BLACK);
            *curLaser = resetLaser;
        }
    }
}

//============================================================================
// update_ties_speed()
//============================================================================
void update_ties_speed() {
    size_t speedWeight = 0;

    if (numRemainingTies <= 1) {
        speedWeight = 495;
        erase_recently_shot_tie_lasers();
    } else if (numRemainingTies <= 2) {
        speedWeight = 490;
        erase_recently_shot_tie_lasers();
    } else if (numRemainingTies <= 3) {
        speedWeight = 475;
        erase_recently_shot_tie_lasers();
    } else if (numRemainingTies <= 6) {
        speedWeight = 400;
    } else if (numRemainingTies <= 9) {
        speedWeight = 300;
    } else if (numRemainingTies <= 12) {
        speedWeight = 200;
    } else if (numRemainingTies <= 15) {
        speedWeight = 100;
    }

    TIM7->ARR = (500 - speedWeight) - 1;
}

//============================================================================
// handle_laser_impact()
//============================================================================
bool handle_laser_impact(Laser* laser) {
    bool hitSomething = false;
    size_t x = laser->x;
    size_t yI = laser->yI;
    size_t yF = laser->yF;

    // Check for hitting the top or bottom borders
    if (yF <= ymin || yF >= ymax) {
        hitSomething = true;
    }
    // Check for hitting shields
    else if ((yI <= shieldsCenterY && shieldsCenterY <= yF) ||
             (yI >= shieldsCenterY && shieldsCenterY >= yF)) {
        size_t shieldHitIdx = 0;
        for (; shieldHitIdx < sizeof(shieldsCenterX)/sizeof(shieldsCenterX[0]);
                shieldHitIdx++) {
            if (shieldsCenterX[shieldHitIdx] - shieldsWidth / 2 <= x &&
                    x <= shieldsCenterX[shieldHitIdx] + shieldsWidth / 2) {
                hitSomething = true;
                break;
            }
        }
        if (hitSomething) {
            LCD_DrawPicture(shieldsCenterX[shieldHitIdx] - shieldsWidth / 2,
                    shieldsCenterY - shieldsHeight / 2, &shieldImg);
        }
    }
    // Check for hitting Millenium Falcon
    else if (yF > falcon.yCenter - falcon.height / 2 &&
             falcon.xCenter - falcon.width / 2 <= x &&
             x <= falcon.xCenter + falcon.width / 2) {
        hitSomething = true;
        gameIsOver = true;
        falcon.image = &falconExplosionImg;
        falcon.state = EXPLODING;
        draw_ship(&falcon);
    }
    // Check for hitting a TIE Fighter
    else if (yF < get_bottom_tie_center_y() + tieImg.height / 2) {
        Ship* curTie;
        for (int row = NUM_ROWS_OF_TIES - 1; row >= 0; row--) {
            for (size_t col = 0; col < NUM_TIES_PER_ROW; col++) {
                curTie = &(ties[row][col]);
                if (curTie->state == ACTIVE &&
                     yF >= curTie->yCenter - tieImg.height / 2 &&
                     curTie->xCenter - tieImg.width / 2 <= x &&
                     x <= curTie->xCenter + tieImg.width / 2) {
                    hitSomething = true;
                    // Break out of double for loop
                    row = -1;
                    break;
                }
            }
        }
        if (hitSomething) {
            curTie->image = &tieExplosionImg;
            curTie->state = EXPLODING;
            update_score(curTie->scoreGain);
            draw_ship(curTie);
            numRemainingTies -= 1;
            if (numRemainingTies <= 0) {
                gameIsOver = true;
            } else {
                update_ties_speed();
            }
        }
    }

    if (hitSomething) {
        LCD_DrawLine(x, yI, x, yF, BLACK);
        *laser = resetLaser;
    }
    return hitSomething;
}

//============================================================================
// handle_lasers()
//============================================================================
void handle_lasers(void) {
    for (int i = 0; i < sizeof(tieShots)/sizeof(tieShots[0]) + 1; i++) {
        Laser* curLaser;
        if (i < sizeof(tieShots)/sizeof(tieShots[0])) {
            curLaser = &(tieShots[i]);
        } else {
            curLaser = &falconShot;
        }

        if (curLaser->color != 0) {
            bool hitSomething = handle_laser_impact(curLaser);
            if (!hitSomething) {
                move_laser(curLaser);
            }
        }
    }
}

//============================================================================
// falcon_shoot()
//============================================================================
void falcon_shoot(char input) {
    if (input == 'D' && falconShot.color == 0) {
        size_t xPos = falcon.xCenter;
        size_t yPos = falcon.yCenter - (falcon.height) / 2 ;
        falconShot = (Laser) {
          .x = xPos,
          .yI = yPos,
          .yF = yPos - LASER_LEN,
          .pixelDy = -5,
          .color = RED,
        };
    }
}

//============================================================================
// get_laser_dy()
//============================================================================
size_t get_laser_dy(void) {
    switch (numRemainingTies) {
        case 1:
            return 5;
        case 2:
            return 4;
        case 3:
            return 3;
        default:
            return 1;
    }
}

//============================================================================
// ties_shoot()
//============================================================================
void ties_shoot(void) {
    for (int i = 0; i < sizeof(tieShots)/sizeof(tieShots[0]); i++) {
        Laser* curLaser = &(tieShots[i]);
        if (curLaser->color == 0) {
            size_t xLeftmost = get_leftmost_tie_center_x();
            size_t xRightmost = get_rightmost_tie_center_x();
            size_t xPos = xLeftmost + random() % (1 + xRightmost - xLeftmost);
            size_t yPos = get_bottom_tie_center_y() + (tieImg.height) / 2;
            *curLaser = (Laser) {
              .x = xPos,
              .yI = yPos,
              .yF = yPos + LASER_LEN,
              .pixelDy = get_laser_dy(),
              .color = GREEN,
            };
        }
    }
}

//============================================================================
// TIM6_DAC_IRQHandler()
// ISR for timer 6 (100 times per second) that handles updates for faster
// game actions including:
// - moving the player ship (the Millenium Falcon)
// - shooting from the player ship
// - handling the movement and impact of laser fire from all ships
// - ending the game if the player ship is shot
//============================================================================
void TIM6_DAC_IRQHandler(void) {
    TIM6->SR &= ~TIM_SR_UIF;
    if (!gameIsOver) {
        char keyInput = get_key();
        move_falcon(keyInput);
        falcon_shoot(keyInput);
        handle_lasers();
    }
}

//============================================================================
// TIM6_DAC_IRQHandler()
// ISR for timer 7 (2 times per second) that handles updates for slower
// game actions including:
// - random shots from the enemy ships
// - moving the enemy ships (TIE fighters)
// - ending the game if the tie fighters reach the falcon's position
//============================================================================
void TIM7_IRQHandler(void) {
    TIM7->SR &= ~TIM_SR_UIF;
    if (!gameIsOver) {
        move_ties();
        ties_shoot();
    }
}

//============================================================================
// init_falcon()
//============================================================================
void init_falcon(void) {
    falcon = (Ship) {
        .image = &falconImg,
        .width = falconImg.width,
        .height = falconImg.height,
        .pixelDx = 1,
        .pixelDy = 0,
        .xCenter = (xmax + xmin) / 2,
        .yCenter = backgroundImg.height - border - falconImg.height / 2,
        .scoreGain = 0,
        .state = ACTIVE
    };
    draw_ship(&falcon);
}

//============================================================================
// init_shields()
//============================================================================
void init_shields(void) {
    shieldsCenterX[0] = 70;
    shieldsCenterX[1] = backgroundImg.width - 70;
    shieldsCenterY = falcon.yCenter - falcon.height;
    LCD_DrawPicture(shieldsCenterX[0] - shieldsWidth / 2,
                    shieldsCenterY - shieldsHeight / 2, &shieldImg);
    LCD_DrawPicture(shieldsCenterX[1] - shieldsWidth / 2,
                    shieldsCenterY - shieldsHeight / 2, &shieldImg);
}

//============================================================================
// init_ties()
//============================================================================
void init_ties(void) {
    for (size_t row = 0; row < NUM_ROWS_OF_TIES; row++) {
        for (size_t col = 0; col < NUM_TIES_PER_ROW; col++) {
            ties[row][col] = (Ship) {
              .image = &tieImg,
              .width = tieImg.width,
              .height = tieImg.height,
              .pixelDx = 2,
              .pixelDy = 0,
              .xCenter = xmin + col * xmax / (NUM_TIES_PER_ROW + 0.5),
              .yCenter =  ymin + row * (tieImg.height + 10),
              .scoreGain = 100 - (row * 25),
              .state = ACTIVE
            };
            draw_ship(&(ties[row][col]));
        }
    }
}

//============================================================================
// start_game()
//============================================================================
void start_game() {
    // Prompt user to start and seed random number generator
    spi_display1("Press # To Start");
    spi_display2("                ");
    char input = 0;
    do {
        input = get_key();
    } while (input != '#');
    srand(TIM2->CNT);

    // Initialize the game state.
    gameIsOver = false;
    score = 0;
    numRemainingTies = NUM_ROWS_OF_TIES * NUM_TIES_PER_ROW;
    tiesMovingRight = true;
    falconShot = resetLaser;
    tieShots[0] = resetLaser;
    tieShots[1] = resetLaser;
    tieShots[2] = resetLaser;

    LCD_DrawPicture(0, 0, &backgroundImg);
    xmin = border + falconImg.width / 2;
    xmax = backgroundImg.width - border - falconImg.width / 2;
    ymin = border + 5 + tieImg.height / 2;
    ymax = backgroundImg.height - border - falconImg.height / 2;
    init_falcon();
    init_shields();
    init_ties();

    // Display the current score on the OLED
    update_score(0);
    spi_display1("                ");
    spi_display1("Blast those TIEs!");


    setup_tim6();
    setup_tim7();
}

//============================================================================
// draw_title_screen()
//============================================================================
void draw_title_screen() {
    LCD_DrawPicture(0, 0, &backgroundImg);
//LCD_DrawString(u16 x,u16 y, u16 fc, u16 bg, const char *p, u8 size, u8 mode);
    LCD_DrawString(LCD_W / 4, 5, WHITE, BLACK, "DARTH INVADERS", 16, 0);
    LCD_DrawString(LCD_W / 4 - 3, 30, WHITE, BLACK, "Zach Ghera 2020", 16, 0);
    LCD_DrawPicture(LCD_W / 4 - 10, 55, &vaderImg);
    LCD_DrawString(border, 55 + vaderImg.height + 10, WHITE, BLUE,
                    "Instructions:", 16, 0);
    LCD_DrawString(border, 55 + vaderImg.height + 30, WHITE, BLACK,
                    "Shoot down all the TIEs before they ", 12, 0);
    LCD_DrawString(border, 55 + vaderImg.height + 45, WHITE, BLACK,
                    "reach the shields! Using the keypad,", 12, 0);
    LCD_DrawString(border, 55 + vaderImg.height + 60, WHITE, BLACK,
                    "press 'D' to shoot as well as '*' and ", 12, 0);
    LCD_DrawString(border, 55 + vaderImg.height + 75, WHITE, BLACK,
                    "'0' to move left and right. You can ", 12, 0);
    LCD_DrawString(border, 55 + vaderImg.height + 90, WHITE, BLACK,
                    "hide behind shields for cover. Warning", 12, 0);
    LCD_DrawString(border, 55 + vaderImg.height + 105, WHITE, BLACK,
                    "TIEs get faster as you shoot more down", 12, 0);
    LCD_DrawString(border, LCD_H - 20, WHITE, RED,
                    "Press '#' To Start", 16, 0);
}

//============================================================================
// draw_game_over_screen()
//============================================================================
void draw_game_over_screen() {
    spi_display1("                ");
    spi_display1("Game Over");

    LCD_DrawPicture(0, 0, &backgroundImg);
    LCD_DrawString(LCD_W / 3, 30, WHITE, BLUE, "GAME OVER", 16, 0);
    char scoreBuffer[20];
    sprintf(scoreBuffer, "Score: %04d", score);
    LCD_DrawString(LCD_W / 3, 50, WHITE, BLACK, scoreBuffer, 16, 0);
    LCD_DrawString(border, LCD_H - 20, WHITE, RED,
                    "Press '#' To Restart", 16, 0);

    // Draw a little picture with the falcon and some TIEs
    falcon.xCenter = LCD_W / 2;
    falcon.yCenter = LCD_H / 2 - 20;
    falcon.image = &falconImg;
    draw_ship(&falcon);
    Ship* tie1 = &(ties[0][0]);
    tie1->image = &tieImg;
    tie1->xCenter = LCD_W / 2 - 40;
    tie1->yCenter = LCD_H / 2 + 50;
    draw_ship(tie1);
    LCD_DrawLine(tie1->xCenter, tie1->yCenter - tie1->height / 2 - 10,
                tie1->xCenter, tie1->yCenter - tie1->height / 2 - 20, GREEN);
    Ship* tie2 = &(ties[0][1]);
    tie2->image = &tieImg;
    tie2->xCenter = LCD_W / 2 + 40;
    tie2->yCenter = LCD_H / 2 + 50;
    draw_ship(tie2);
    LCD_DrawLine(tie2->xCenter, tie2->yCenter - tie2->height / 2 - 30,
                tie2->xCenter, tie2->yCenter - tie2->height / 2 - 40, GREEN);

    char input = 0;
    do {
        input = get_key();
    } while (input != '#');
    nano_wait(100000000); // wait 0.1 sec
}

int main(void) {
    // Initialize Peripherals/Hardware
    setup_tim2();
    setup_portb();
    setup_spi1();
    LCD_Init();
    setup_spi2();
    spi_init_oled();

    while (true) {
        draw_title_screen();

        start_game();

        while(!gameIsOver);
        // Disable all game actions
        TIM6->CR1 &= ~TIM_CR1_CEN;
        TIM7->CR1 &= ~TIM_CR1_CEN;
         nano_wait(1000000000); // wait 1 sec

        draw_game_over_screen();
    }

}
