//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//

// Budget limit: 64Kbits + 1024 bits
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Diyou Wang";
const char *studentID = "A17118730";
const char *email = "diw011@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = {"Static", "Gshare",
                         "Tournament", "Custom"};

// define number of bits required for indexing the BHT here.
int ghistoryBits = 15; // Number of bits used for Global History
int bpType;            // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
// TODO: Add your own Branch Predictor data structures here
//
// gshare
uint8_t *bht_gshare;
uint64_t ghistory;

// tournament sizes
int pcIndexBits = 10;  // Number of bits used for PC index
int lhistoryBits = 10; // Number of bits used for Local History
int tghistoryBits = 12; // Number of bits used for Tournament Global History
int phistoryBits = 12;  // Number of bits used for Path History
int choiceBits = 12; // Number of bits used for Choice Predictor

// Tables
uint8_t *bht_local; // local predictor 3 bit counter
uint16_t *lht_local; // local history table 10 bits history
uint8_t *tbht_global; // tournament global predictor 2 bit counter
uint8_t *bht_choice; // choice predictor 2 bit counter
uint16_t path_history; // path history register
//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor

//tournament functions
void init_tournament()
{
  int lht_entries = 1 << pcIndexBits;
  int bht_local_entries = 1 << lhistoryBits;
  int tbht_global_entries = 1 << tghistoryBits;
  int bht_choice_entries = 1 << tghistoryBits;

  // allocate memory
  lht_local = (uint16_t *)malloc(lht_entries * sizeof(uint16_t));
  bht_local = (uint8_t *)malloc(bht_local_entries * sizeof(uint8_t));
  tbht_global = (uint8_t *)malloc(tbht_global_entries * sizeof(uint8_t));
  bht_choice = (uint8_t *)malloc(bht_choice_entries * sizeof(uint8_t));

  // initialize tables
  for (int i = 0; i < lht_entries; i++)
  {
    lht_local[i] = 0;
  }
  for (int i = 0; i < bht_local_entries; i++)
  {
    bht_local[i] = WN;
  }
  for (int i = 0; i < tbht_global_entries; i++)
  {
    tbht_global[i] = WN;
    bht_choice[i] = WN;
  }

  ghistory = 0; // idk about this one
  path_history = 0;
}

uint8_t tournament_predict(uint32_t pc)
{
  
  return NOTTAKEN;
}

void train_tournament(uint32_t pc, uint8_t outcome)
{
  // Update local predictor
  uint32_t lht_index = pc & ((1 << pcIndexBits) - 1);
  uint16_t local_history = lht_local[lht_index];
  uint32_t bht_local_index = local_history & ((1 << lhistoryBits) - 1);

  // Update state of entry in local BHT based on outcome
  switch (bht_local[bht_local_index])
  {
  case WN:
    bht_local[bht_local_index] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    bht_local[bht_local_index] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    bht_local[bht_local_index] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    bht_local[bht_local_index] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in Local BHT!\n");
    break;
  }

  // Update local history table
  lht_local[lht_index] = ((local_history << 1) | outcome) & ((1 << lhistoryBits) - 1);
}

void cleanup_tournament()
{
  free(lht_local);
  free(bht_local);
  free(tbht_global);
  free(bht_choice);
}


// gshare functions
void init_gshare()
{
  int bht_entries = 1 << ghistoryBits;
  bht_gshare = (uint8_t *)malloc(bht_entries * sizeof(uint8_t));
  int i = 0;
  for (i = 0; i < bht_entries; i++)
  {
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}

uint8_t gshare_predict(uint32_t pc)
{
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;
  switch (bht_gshare[index])
  {
  case WN:
    return NOTTAKEN;
  case SN:
    return NOTTAKEN;
  case WT:
    return TAKEN;
  case ST:
    return TAKEN;
  default:
    printf("Warning: Undefined state of entry in GSHARE BHT!\n");
    return NOTTAKEN;
  }
}

void train_gshare(uint32_t pc, uint8_t outcome)
{
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;

  // Update state of entry in bht based on outcome
  switch (bht_gshare[index])
  {
  case WN:
    bht_gshare[index] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    bht_gshare[index] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    bht_gshare[index] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    bht_gshare[index] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in GSHARE BHT!\n");
    break;
  }

  // Update history register
  ghistory = ((ghistory << 1) | outcome);
}

void cleanup_gshare()
{
  free(bht_gshare);
}

void init_predictor()
{
  switch (bpType)
  {
  case STATIC:
    break;
  case GSHARE:
    init_gshare();
    break;
  case TOURNAMENT:
    break;
  case CUSTOM:
    break;
  default:
    break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint32_t make_prediction(uint32_t pc, uint32_t target, uint32_t direct)
{

  // Make a prediction based on the bpType
  switch (bpType)
  {
  case STATIC:
    return TAKEN;
  case GSHARE:
    return gshare_predict(pc);
  case TOURNAMENT:
    return NOTTAKEN;
  case CUSTOM:
    return NOTTAKEN;
  default:
    break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

void train_predictor(uint32_t pc, uint32_t target, uint32_t outcome, uint32_t condition, uint32_t call, uint32_t ret, uint32_t direct)
{
  if (condition)
  {
    switch (bpType)
    {
    case STATIC:
      return;
    case GSHARE:
      return train_gshare(pc, outcome);
    case TOURNAMENT:
      return;
    case CUSTOM:
      return;
    default:
      break;
    }
  }
}
